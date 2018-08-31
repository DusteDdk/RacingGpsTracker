// License: WTFPL
// This is not only my first Go program, I also made no effort to make it pretty, so, enjoy.
// Build with: go build
// Run with ./framer --help

// Why framer?
// Because it outputs frames.

package main

import (
	"flag"
	"fmt"
	"image"
	"image/color"
	"image/png"
	"io/ioutil"
	"math"
	"os"
	"strings"
	"time"

	"container/ring"

	"github.com/adrianmo/go-nmea"
	"github.com/llgcode/draw2d"
	"github.com/llgcode/draw2d/draw2dimg"
	"github.com/llgcode/draw2d/draw2dkit"
)

// Point - A point indeed
type Point struct {
	x float64
	y float64
}

// Line - Another representation of a line
type Line struct {
	a, b Point
}

// I totally failed at implementing this again, so I just cargo-culted it from OSGG (and I probably didn't understand it back then either)
// It's not good enough that I can get the actual point of intersection, but then again, I'd probably fail at interpolating to gain extra precision anyway..
func linesCross(a, b Line) bool {

	x0 := a.a.x
	y0 := a.a.y
	x1 := a.b.x
	y1 := a.b.y
	x2 := b.a.x
	y2 := b.a.y
	x3 := b.b.x
	y3 := b.b.y

	d := (x1-x0)*(y3-y2) - (y1-y0)*(x3-x2)
	if math.Abs(d) < 0.001 {
		return false
	}

	AB := ((y0-y2)*(x3-x2) - (x0-x2)*(y3-y2)) / d

	if AB > 0.0 && AB < 1.0 {
		CD := ((y0-y2)*(x1-x0) - (x0-x2)*(y1-y0)) / d
		if CD > 0.0 && CD < 1.0 {
			return true
		}
	}

	return false
}

func encode(frame *image.RGBA, curFrameNum int, outDir string) {
	f, err := os.Create(fmt.Sprintf("%s/frame%06d.png", outDir, curFrameNum))
	if err != nil {
		panic(err)
	}

	if err := png.Encode(f, frame); err != nil {
		f.Close()
		panic(err)
	}

	if err := f.Close(); err != nil {
		panic(err)
	}
}

func toxy(m nmea.GPRMC, s float64) (x float64, y float64) {

	y = (180.0 + m.Latitude) / (s * 360.0)
	x = (90.0 - m.Longitude) / (s * 180.0)
	return
}

func dot(ctx draw2d.GraphicContext, x float64, y float64) {

	draw2dkit.Circle(ctx, x, y, 7)
	ctx.SetStrokeColor(color.RGBA{0, 0, 0, 255})
	ctx.SetLineWidth(6)
	ctx.Stroke()

	draw2dkit.Circle(ctx, x, y, 4)
	ctx.SetStrokeColor(color.RGBA{255, 0, 0, 255})
	ctx.SetLineWidth(4)
	ctx.Stroke()

	draw2dkit.Circle(ctx, x, y, 2)
	ctx.SetStrokeColor(color.RGBA{255, 0, 0, 255})
	ctx.SetLineWidth(4)
	ctx.Stroke()

}

func outline(ctx draw2d.GraphicContext, x float64, y float64, col color.Color, text string, size float64) {

	x = x + 3
	y = y + 3
	ctx.SetFontSize(size)
	ctx.SetFillColor(color.RGBA{0, 0, 0, 255})
	ctx.FillStringAt(text, x-3, y)
	ctx.FillStringAt(text, x, y-3)
	ctx.FillStringAt(text, x-3, y-3)
	ctx.FillStringAt(text, x+3, y-3)

	ctx.FillStringAt(text, x+3, y)
	ctx.FillStringAt(text, x, y+3)
	ctx.FillStringAt(text, x+3, y+3)
	ctx.FillStringAt(text, x-3, y+3)

	ctx.SetFillColor(col)
	ctx.FillStringAt(text, x, y)
}

const mapSize = 500.0

func unsetMapCoords(ctx draw2d.GraphicContext) {
	ctx.Rotate((1.570796 * 2) * -1)
	ctx.Translate(-mapSize-10, -mapSize-10)
}

func setMapCoords(ctx draw2d.GraphicContext) {
	ctx.Translate(+mapSize+10, +mapSize+10)
	ctx.Rotate(1.570796 * 2)
}

func main() {
	fmt.Println("Framer")

	firstSample := flag.Int("first", 0, "Start at this sample (optional)")
	lastSample := flag.Int("last", -1, "Stop at this sample (optional)")

	firstTrackSample := flag.Int("trackFirst", 0, "Start track at this sample (optional)")
	lastTrackSample := flag.Int("trackLast", -1, "Stop track at this sample (optional)")

	showOutline := flag.Bool("outline", false, "show the map rendering outline (optional)")
	inFile := flag.String("in", "", "RMC file to read coordinates from (REQUIRED)")
	outDir := flag.String("out", "", "Directory in which to write frames, will overwrite! (REQUIRED)")
	onlyThisFrame := flag.Int("onlyFrame", -1, "Only output this frame (optional)")

	renderFrameNum := flag.Bool("showFrameNumber", false, "Add the frame (sample) number as red text at bottom right (optional)")
	finishLineSample := flag.Int("finishAt", 0, "Put a finish-line at this sample  (optional)")
	finishLineAngle := flag.Float64("finishAngle", 0, "The angle of the finish line (if using finishLineAt) in degrees")
	finishLineWidth := flag.Float64("finishWidth", 15, "The width of the finish-line (tweak this)")

	firstLap := flag.Int("firstLap", 0, "Don't count crossing finishLine the first N times  (optional)")
	lastLap := flag.Int("lastLap", 100, "Don't count crossing finishLine after N times (optional)")

	speedGraph := flag.Bool("speedGraph", true, "Show the speed-graph")

	flag.Parse()

	if *inFile == "" || *outDir == "" {
		fmt.Println("-in FILE and -out DIR are required, see --help for more info.")
		os.Exit(1)
	}

	const height = 1080.0
	const width = 1920.0

	const mapPosX = 5.0
	const mapPosY = 5.0

	const fps = 59.94
	const frameTime = 1000.0 / fps
	const gpsHz = 10.0
	const framesPrGpsSample = gpsHz / fps

	videoTime := 0.0

	start := time.Now()

	data, err := ioutil.ReadFile(*inFile)

	if err != nil {
		panic(err)
	}

	text := string(data)

	arr := strings.Split(text, "|")

	fmt.Println("There are ", len(arr), " samples.")

	if *lastSample < 0 {
		*lastSample = len(arr)
	}

	fmt.Println("Using samples [", *firstSample, "..", *lastSample, "].")

	arr = arr[*firstSample:*lastSample]

	numSamples := len(arr)
	samples := make([](nmea.GPRMC), numSamples)

	if *firstTrackSample == -1 {
		*firstTrackSample = 0
	}
	if *lastTrackSample == -1 {
		*lastTrackSample = numSamples - 1
	}

	minX := width
	maxX := 0.0
	minY := height
	maxY := 0.0

	for idx, line := range arr {

		// To save space/time/wear, the firmware cuts the first part of the RMC message before writing it to the file on the SD card.
		// First thing, reconstruct the original message and use some nmea lib to parse it, because lazy
		origMsg := "$GPRMC," + line

		s, err := nmea.Parse(origMsg)
		if err == nil {

			m := s.(nmea.GPRMC)

			samples[idx] = m

			videoTime += 10.0

			if m.Validity == "A" {

				x, y := toxy(m, mapSize)

				if x < minX {
					minX = x
				}
				if x > maxX {
					maxX = x
				}

				if y < minY {
					minY = y
				}
				if y > maxY {
					maxY = y
				}

			}

		} else {
			fmt.Println("Invalid sample ", idx, ":", line, "err:", err)
		}

	}

	frame := image.NewRGBA(image.Rectangle{Max: image.Point{X: width, Y: height}})

	fgc := draw2dimg.NewGraphicContext(frame)

	maxSpeed := 0.0

	scaleX := mapSize / (maxX - minX)
	scaleY := mapSize / (maxY - minY)

	var finishLine Line

	fgc.BeginPath()

	for i, val := range samples {

		validSample := (val.Validity == "A")
		if validSample && i >= *firstTrackSample && i <= *lastTrackSample {

			x, y := toxy(val, mapSize)

			x = (x - minX) * scaleX
			y = (y - minY) * scaleY

			if val.Speed > maxSpeed {
				maxSpeed = val.Speed
			}

			if i != 0 {
				fgc.LineTo(x, y)

				if i == *finishLineSample {
					flw := *finishLineWidth
					fla := *finishLineAngle

					a := Point{x + math.Cos(fla*0.0174532925)*flw, y + math.Sin(fla*0.0174532925)*flw}
					b := Point{x + math.Cos((fla+180)*0.0174532925)*flw, y + math.Sin((fla+180)*0.0174532925)*flw}
					finishLine = Line{a, b}
				}

			} else {
				fgc.MoveTo(x, y)
			}
		}
	}

	maxSpeed *= 1.852

	setMapCoords(fgc)

	path := fgc.GetPath()

	fgc.SetStrokeColor(color.RGBA{0, 0, 0, 0xff})
	fgc.SetLineWidth(14)
	fgc.Stroke()

	fgc.SetStrokeColor(color.RGBA{255, 255, 255, 0xff})
	fgc.SetLineWidth(5)

	fgc.Stroke(&path)

	if *showOutline {
		fgc.BeginPath()
		fgc.SetStrokeColor(color.RGBA{255, 0, 0, 0xff})
		fgc.SetLineWidth(1)
		fgc.MoveTo(0, 0)
		fgc.LineTo(mapSize, 0)
		fgc.LineTo(mapSize, mapSize)
		fgc.LineTo(0, mapSize)
		fgc.LineTo(0, 0)
		fgc.Stroke()
	}

	if *finishLineSample > 0 {
		fgc.BeginPath()
		fgc.SetStrokeColor(color.RGBA{0, 0, 0, 0xff})
		fgc.SetLineWidth(6)
		fgc.MoveTo(finishLine.a.x, finishLine.a.y)
		fgc.LineTo(finishLine.b.x, finishLine.b.y)
		fgc.Stroke()

		fgc.BeginPath()
		fgc.SetStrokeColor(color.RGBA{0, 0, 255, 0xff})
		fgc.SetLineWidth(3)
		fgc.MoveTo(finishLine.a.x, finishLine.a.y)
		fgc.LineTo(finishLine.b.x, finishLine.b.y)
		fgc.Stroke()
	}

	// Setup text stuff
	fgc.SetFontData(draw2d.FontData{Name: "DejaVu", Family: draw2d.FontFamilySerif, Style: draw2d.FontStyleNormal})

	// Save the background
	bgPix := make([]uint8, frame.Bounds().Max.X*frame.Bounds().Max.Y*4)
	copy(bgPix, frame.Pix)

	// Render frames
	unsetMapCoords(fgc)

	px := 0.0
	py := 0.0
	x := 0.0
	y := 0.0

	overFinish := 0
	curLap := 0 - *firstLap

	curSpeed := 0.0

	graphHeight := 150.0
	graphWidth := 400.0

	const numGraphSamples = 200
	speedRing := ring.New(numGraphSamples)

	for i := 0; i < numGraphSamples; i++ {
		speedRing.Value = graphHeight - 1
		speedRing = speedRing.Next()
	}

	// Final iteration
	for cf, val := range samples {

		validSample := (val.Validity == "A")

		// Extract
		if validSample {

			x, y = toxy(val, mapSize)
			x = (x - minX) * scaleX
			y = (y - minY) * scaleY

			// Handle start line intersection
			if px != 0.0 && py != 0 {
				testLine := Line{Point{x, y}, Point{px, py}}

				if linesCross(finishLine, testLine) {
					if overFinish > 0 {
						//Stamp it, next!
						// Todo: Consider not doing that, and draw all the times every frame so we can slide them out as new times pop in.
						if curLap > -1 && curLap < *lastLap {

							ms := (cf - overFinish) * 100
							minutes := ms / 1000 / 60
							seconds := ms / 1000 % 60
							ms = ms % 1000
							ms = ms / 100

							copy(frame.Pix, bgPix)
							outline(fgc, width/2+750, 60+60*float64(curLap), color.RGBA{255, 255, 255, 255}, fmt.Sprintf("%0d:%02d.%d", minutes, seconds, ms), 40)
							copy(bgPix, frame.Pix)
						}
						curLap++

					}
					overFinish = cf
				}
			}
			px = x
			py = y
		}

		// Render frames
		if *onlyThisFrame == -1 || cf == *onlyThisFrame {
			fmt.Printf("Frame: %d / %d\n", cf, numSamples)

			if validSample {

				copy(frame.Pix, bgPix)

				setMapCoords(fgc)
				dot(fgc, x, y)
				unsetMapCoords(fgc)
				// Display current speed
				curSpeed = val.Speed * 1.852
				outline(fgc, width/2-372/2+20, height-20, color.RGBA{255, 255, 255, 255}, fmt.Sprintf("%05.1f km/h", curSpeed), 70)

				//Display the speed-graph
				if *speedGraph {

					fgc.Translate(1510, 920)
					fgc.BeginPath()
					fgc.SetFillColor(color.RGBA{0, 0, 0, 64})
					fgc.SetLineWidth(1)
					fgc.MoveTo(0, 0)
					fgc.LineTo(graphWidth, 0)
					fgc.LineTo(graphWidth, graphHeight)
					fgc.LineTo(0, graphHeight)
					fgc.LineTo(0, 0)
					path := fgc.GetPath()

					fgc.Fill()

					speedRing.Value = (curSpeed/maxSpeed)*-graphHeight + graphHeight
					speedRing = speedRing.Next()

					fgc.BeginPath()
					fgc.SetStrokeColor(color.RGBA{0xff, 0xff, 0xff, 0xff})
					fgc.SetLineWidth(2)
					fgc.MoveTo(0, speedRing.Value.(float64))

					sw := graphWidth / numGraphSamples
					for i := 0; i < numGraphSamples; i++ {
						fgc.LineTo(float64(i)*sw+sw, speedRing.Value.(float64))
						speedRing = speedRing.Next()
					}
					fgc.Stroke()

					fgc.SetStrokeColor(color.RGBA{0, 0, 0, 0xff})
					fgc.SetLineWidth(1)
					fgc.Stroke(&path)

					msg := fmt.Sprintf("%.1f", maxSpeed)
					fgc.SetFontSize(20)
					fgc.SetFillColor(color.RGBA{255, 255, 255, 255})
					_, h, r, _ := fgc.GetStringBounds(msg)
					fgc.FillStringAt(msg, graphWidth-r-2, -h+2)

					fgc.Translate(-1510, -920) // I REGRET NOTHING! If it had a translation stack I'd not have to resort to such violence.
				}

				// Display current lap
				if overFinish > 0 {

					if curLap > -1 && curLap < *lastLap {

						outline(fgc, 10, height-20, color.RGBA{255, 255, 255, 255}, fmt.Sprintf("Lap %d", curLap), 40)
						ms := (cf - overFinish) * 100
						minutes := ms / 1000 / 60
						seconds := ms / 1000 % 60
						ms = ms % 1000
						ms = ms / 100

						outline(fgc, width/2+750, 60+60*float64(curLap), color.RGBA{255, 255, 255, 255}, fmt.Sprintf("%0d:%02d.%d", minutes, seconds, ms), 40)

					}

				}

			}

			// Display frame number
			if *renderFrameNum {
				msg := fmt.Sprint(cf)
				fgc.SetFontSize(20)
				fgc.SetFillColor(color.RGBA{255, 0, 0, 255})
				fgc.FillStringAt(msg, 10, height-6)
			}

			encode(frame, cf, *outDir)
		}

	}

	stop := time.Now()

	dur := stop.Sub(start)
	fmt.Printf("Program ran for %d ms\n", dur.Nanoseconds()/1000000)

}
