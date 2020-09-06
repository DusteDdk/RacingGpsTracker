const fn = process.argv[2];
if(!fn || fn === '--help' || fn === '-h') {
    console.error('Take a JSON gps stream and generate truncated NMEA sentences for use with the goFramer');
    console.error('required: json file to transform');
    process.exit(1);
}
const data = require(fn);
/*

{
    "1": {
        "streams": {
            "GPS5": {
                "samples": [
                    {
                        "value": [
                            56.3392667,
                            10.6802929,
                            16.05,
                            0,
                            0
                        ],
                        "cts": 41.542,
                        "date": "2020-09-05T06:05:40.349Z",
                        "sticky": {
                            "fix": 0,
                            "precision": 9999,
                            "altitude system": "MSLV"
                        }
                    },


 * */

let last = 0;
let num=0;
let active = 'V';
let str='';
let lastActive='';
let frameLength=0;
let time=100;
data['1'].streams.GPS5.samples.forEach( (sample, sampleIdx, samples)=>{

    const now = new Date(sample.date);
    if(!last) {
        last = new Date(sample.date).getTime();
    } else {

        last = now.getTime();
    }
    num++;
    const delta = now.getTime() - last;
    const h = (now.getUTCHours()<10)?'0'+now.getUTCHours():now.getUTCHours();
    const m = (now.getUTCMinutes()<10)?'0'+now.getUTCMinutes():now.getUTCMinutes();
    const s = (now.getUTCSeconds()<10)?'0'+now.getUTCSeconds():now.getUTCSeconds();
    const ms = Math.floor(now.getUTCMilliseconds()/10);
    const pms = (ms <10)? '0'+ms:ms;
    const rmcTime = `${h}${m}${s}.${pms}`;
    if(sample.sticky) {
        if(sample.sticky.fix === 0) {
            active = 'V';
        }
        if(sample.sticky.fix > 0) {
            active = 'A';
        }
    }


    const dlat = sample.value[0];
    const dlon = sample.value[1]/2.0;

    const latd = Math.floor(dlat);
    const latm = (dlat - latd) * 60;


    const rmcLat = `${latd}${latm.toFixed(5)},N`;

    const lond = Math.floor(dlon);
    const lonm = (dlon - lond) * 60;

    const rmcLon = `${lond}${lonm.toFixed(5)},E`;


//    const rmcLat = `${dlat},N`;
//    const rmcLon = `${dlon},E`;

    const speed = (sample.value[3] * 3.6).toFixed(3);

    let day = now.getUTCDay();
    if(day<10) {
        day = '0'+day;
    }
    let month = now.getUTCMonth();
    if(month < 10) {
        month = '0'+month;
    }
    let year = ''+now.getUTCFullYear();
    year = (year[2]+year[3]);
    const rmcDate = `${day}${month}${year}`;

    const rmcSpeed = (speed * 0.539957).toFixed(3);

    let rmc = `GPRMC,${rmcTime},${active},${rmcLat},${rmcLon},${rmcSpeed},,${rmcDate},,,A`;
    let chk = 0;
    for(let i = 0; i < rmc.length; i++) {
        chk ^= rmc.charCodeAt(i);
    }
    if(true || active === 'A') {
        lastActive = rmc.substring(6,rmc.lenght)+'*'+chk.toString(16).toUpperCase()+'|';
        if( sampleIdx+1 < samples.length ) {
            frameLength = samples[sampleIdx+1].cts - sample.cts;
        }
    }

    // roughest downsampling to 10 hz.
    time += frameLength;
    if(time >= 100) {
        time -= 100;
        str += lastActive
    } 

});

console.log(str);
