    // 
    
    batLength=72;
    batDia=18.25;
    module battery() {
        rotate( [ 0,90,0 ] )
        cylinder( batLength, batDia/2, batDia/2);
    }
    
    module batteries() {
        color( [0.5,0.5,1] ) {
            translate( [0,batDia/2+0.25,0 ] )
            battery();
            translate( [0,-batDia/2-0.25,0 ] )
            battery();
        }
    } 
    
    
    
    gpsBoardWidth=26.4;
    gpsBoardHeight=35.7;
    gpsBoardDepth=3.7;
    gpsAntOffset=4.9; // from bottom
    gpsAntWidth=25.5;
    gpsAntHeight=25.5;
    gpsAntDepth=9.6;
    module gpsModule(antSpace) {
        
        color([1,1,0] )
        difference() {
            cube([gpsBoardWidth, gpsBoardHeight, 1.3] );
    
            translate( [1.5+1.3, 1+1.5, -1 ] )
            cylinder(10, 1.5, 1.5);
            translate( [1.5+1.3, gpsBoardHeight-1.5-1.5, -1 ] )
            cylinder(10, 1.5, 1.5);
    
            translate( [gpsBoardWidth-1.5-1.8, 1+1.5, -1 ] )
            cylinder(10, 1.5, 1.5);
            translate( [gpsBoardWidth-1.5-1.8, gpsBoardHeight-1.5-1.5, -1 ] )
            cylinder(10, 1.5, 1.5);
    
        }
        translate( [ 0, gpsAntOffset, 1.3 ] )
        color([1,0,0] )
        cube([gpsBoardWidth, gpsAntHeight, gpsAntDepth] );
    
        if(antSpace) {
            translate( [ -2, gpsAntOffset-2, 1.3 ] )
            color([1,1,0] )
            cube([gpsBoardWidth+4, gpsAntHeight+4, gpsAntDepth+2] );
        }
    }
    
    
    
    psuWidth=38.2;
    psuHeight=5.8;
    psuDepth=9.7;
    module psuPcb() {
        color([1,0,0])
        cube( [psuWidth, psuHeight, psuDepth] );
    }
    
    
    mcuWidth=33.1;
    mcuHeight=4.9;
    mcuDepth=18.3;
    
    mcuUsbOffset=5.4;
    mcuUsbHeight=3.5;
    mcuUsbDepth=8.5;
    
    module mcuPcb() {
        cube( [mcuWidth, mcuHeight, mcuDepth ] );
        translate( mcuPcbOffset )
        translate( [mcuWidth-1,0,mcuUsbOffset])
        cube( [10, mcuUsbHeight, mcuUsbDepth ] );
    }
    
    sdWidth=38.2;
    sdHeight=7;
    sdDepth=18;
    sdSlotOffset=1.6;
    sdSlotDepth=13.5;
    sdSlotHeight=2.7;
    module sdPcb() {
        cube( [sdWidth, sdHeight, sdDepth ] );
        translate( sdOffset )
        translate( [sdWidth-1,0,sdSlotOffset] )
        cube( [  10, sdSlotHeight, sdSlotDepth ] );
    }
    
    swWidth=14;
    swHeight=13.05;
    swDepth=7;
    swDia=6;
    module switch() {
        color( [ 1,1,0 ] )
        cube( [swWidth, swHeight, swDepth ] );
        
        translate(swOffset)
        translate( [10, swHeight/2,swDepth/2 ] )
        rotate( [0,90,0 ] )
        cylinder( 10, swDia/2, swDia/2 );
    }
    
    btnWidth=6.18;
    btnDepth=4;
    btnDia=3.8;
    module btn() {
        cube( [btnWidth, btnWidth, btnDepth] );
        translate([ btnWidth/2, btnWidth/2, btnDepth])
        cylinder( 10, btnDia/2, btnDia/2 );
    }
    
    
    module gpsmnt(h,r) {
        
        translate( [1.5+1.3, 1+1.5, -1 ] )
        cylinder(h, r, r);
        translate( [1.5+1.3, gpsBoardHeight-1.5-1.5, -1 ] )
        cylinder(h, r, r);
    
        translate( [gpsBoardWidth-1.5-1.8, 1+1.5, -1 ] )
        cylinder(h, r, r);
        translate( [gpsBoardWidth-1.5-1.8, gpsBoardHeight-1.5-1.5, -1 ] )
        cylinder(h, r, r);
       
    }
    
    w=40;
    $fn=90;
    module topPlate() {
        
        translate([-4,-3,-3]) difference() {
            cube( [batLength, w, 3] );    
            translate([-1,2,1.4]) cube( [batLength+2, w-4, 3] );    
            
            translate([0,21,-3]) cylinder( 10,6,6 );
            translate([batLength,w/2,-3]) cylinder( 10,6,6 );
    
    
        }
    }
    //btn();
    module batbox() {
        difference() {
            translate([0, -w/2,-batDia/2-0.3])
            cube([batLength, w, batDia/2+0.3]);
            batteries();
        }
        
        translate([0, -w/2,0])
        cube([batLength, 1.4, batDia/2+0.2]);
        translate([0, -1.4+w/2,0])
        cube([batLength, 1.4, batDia/2+0.2]);
    }
    
    module rails() {    
            translate( [10,0,0] ) cube([1.5,1.5,21.725+15]);
            translate( [50,0,0] ) cube([1.5,1.5,21.725+15]);
            translate( [30,40-1.5,0] ) cube([1.5,1.5,21.725+15]);
    }
    
    module rails2() {    
    
            translate( [10-0.03,0,0] ) cube([1.6,1.6,21.725+15]);
    
            translate( [50,0,0] ) cube([1.5,1.5,21.725+15]);
    
            translate( [50-0.03,0,0] ) cube([1.6,1.6,21.725+15]);
    
            translate( [30-0.03,40-1.6,0] ) cube([1.6,1.6,21.725+15]);
    
    }
    
    
    module middle()  {
        difference() {
            topPlate();
            translate([0,-3,-21.725]) rails2();
        }
    
        translate([0,-0.5,-2]) gpsmnt(8, 1.5);
        translate([0,-0.5,-2]) gpsmnt(3+2, 2.0);
    }
    
    
    
    module bottom() {
    
    
    difference() {
        translate([-4,17,-12.3]) batbox();
        translate([0,-3,-21.725]) rails();
    
    }
    
    difference() {
        color([1,0,0]) translate([-6,-4,-12.3]) translate([-10,0,-10.425]) cube( [76+20,42, 1] );
        studs();
    }
    
    }
    
    
    //translate([-4,17,-12.3]) batteries();
    
    //translate([0,-0.5,2.0 ]) gpsModule(false);
    
    //    topPlate();
    
    //translate([0,-0.5,2.0 ]) gpsModule(false);
      //translate([-6,-4,-21.725]) shell();
    
    
    module shell() {
        cube( [76,42,36.9] );
        translate([-10,0,0]) cube( [76+20,42, 5] );
        
    }
    
    module studs() {
        translate([-11,5,-25]) cylinder(10,2.55, 2.55);
        translate([-11,30,-25]) cylinder(10,2.55, 2.55);
        translate([75,5,-25]) cylinder(10,2.55, 2.55);
        translate([75,30,-25]) cylinder(10,2.55, 2.55);
    }
    
    
    
    module top() {
    difference() {
        translate([-6,-4,-21.725]) shell();
        translate([-4,-3,-21.725-1]) cube( [72,40,36.9] );
        translate([0,-0.5,2.0 ]) gpsModule(true);
        translate([55,3,10.5]) btn();
        translate([55+3, 42/2-6, 10.5]) cylinder(100,2.5,2.5);
        translate([55+3, 42/2-6+6, 10.5]) cylinder(100,2.5,2.5);
        translate([55+3, 42/2-6+6+6, 10.5]) cylinder(100,2.5,2.5);
        translate([54,22,3]) rotate([0,0,0]) switch();
        studs();
        
    }
    
      difference() {
        translate([-4,-3,-3]) difference() {
            color([1,0,0]) cube( [batLength, w, 18.1] );    
            translate([-1,1.5,-1]) cube( [batLength+2, w-3, 100] );    
    
        }
    
    
        translate([-4,-3,-3]) difference() {
            cube( [batLength, w, 3] );    
            translate([-1,1.5,-1]) cube( [batLength+2, w-3, 10] );    
    
        }
    }
    
    translate([0,-3,-21.725]) rails();
    
    difference() {
        translate([0,-0.5,4.4]) gpsmnt(3+4+4.77, 2.0);
        translate([0,-0.5,-1]) gpsmnt(8, 1.55);
    }
    
    
    difference() {
        translate([55-1,2,12]) cube([btnWidth+2,btnWidth+2, 3.0]);
        translate([55,3,10.5]) btn();
    }
    }


// Here are the parts, render one at a time!
// Remove the translations if you like.. just there because they look cool
translate([100,0,0 ]) top();
translate([0,50,0 ] ) middle();
bottom();

