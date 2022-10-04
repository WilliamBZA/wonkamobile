//http://customizer.makerbot.com/docs

/* [Nr.Spokes] */
raggi=5;			//[3:30]
/* [Wheel Diameter] */
wheel_diameter=100;	//[20:0.1:120]
/* [O-ring] */
oring=0;			//[0,1]
/* [Fudge factor] */
fudge_factor = 0;	//[0:10]
height = 16;
/* [Definition] */
fn=100; 			//[50:500]
difference(){
	union()	{
		//rim
        difference () {
            cylinder (r=wheel_diameter / 2, h=height, $fn=fn);
            translate([0,0,-0.1]) cylinder (r=(wheel_diameter / 2)-1.8, h=height + 0.2, $fn=fn);
        }
        // add spokes (circular cut-outs) raggi       
        for(i = [0:raggi]) rotate([0, 0, i * 360 / raggi+1]) translate([-0.9, 0, 0]) cube([1.8, (wheel_diameter/2)-0.9, height]);
		//boss
		cylinder(r=12 / 2, h=height + 6, $fn=fn);
        //profilo interno
        translate([0, 0, 0]) rotate_extrude(convexity=10, $fn=fn) translate([(wheel_diameter / 2)-1.8, height/2, 0]) rotate([0,0,45]) square([height/sqrt(2),height/sqrt(2)],center=true);
	}
	union()	{
		//shaft cutout (shaft with flat)
		translate([0, 0, 0.6]) intersection(){
			cylinder(r=5.5 / 2 + fudge_factor, h=height+6, $fn=fn*2/5); // was 5
			translate([0, 0, height]) cube([3.0 + fudge_factor, height+8, 12], center=true); // was 2.5
		}
		// set screw for M3
		translate([0, 0, height+3]) rotate([90, 0, 90]) cylinder(r=1.5, h=60, center=true, $fn=fn*2/5); // was 1
		//rim groove cutout
		translate([0, 0, height/2]) rotate_extrude(convexity=10, $fn=fn) translate([wheel_diameter / 2, 0, 10]) rotate([0,0,45]) square([(height - 2)/sqrt(2),(height - 2)/sqrt(2)],center=true);
	}
}

// 16 == 10 
// 13 == 8
// 10 == 6
//  6 == 3