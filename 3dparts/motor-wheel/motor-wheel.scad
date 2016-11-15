$fn = 256;

innerHeight = 20;
outerHeight = 20;

innerDiam1 = 8.5;
innerDiam2 = 9.55;
outerDiam = 100;

difference() {
    rotate_extrude(angle=360,convexity=2)
    polygon(
        points = [
            [0,             0],
            [0,             innerHeight/2-2],
            [outerDiam/2,   outerHeight/2],
            [outerDiam/2+2, outerHeight/2],
            [outerDiam/2+2, outerHeight/2-2],
            [outerDiam/2,   outerHeight/2-2],
            [outerDiam/2,   0],

            [outerDiam/2,   -1*(outerHeight/2-2)],
            [outerDiam/2+2, -1*(outerHeight/2-2)],
            [outerDiam/2+2, -1*(outerHeight/2)],
            [outerDiam/2,   -1*(outerHeight/2)],
            [0,             -1*(outerHeight/2)],
        ]
    );
    difference() {
        cylinder(h=innerHeight, d=innerDiam2,center=true);
        translate([-1*innerDiam2/2,innerDiam1/2,-1*innerHeight/2]) cube(size=[innerDiam2,innerDiam2,innerHeight]);
    }
}
