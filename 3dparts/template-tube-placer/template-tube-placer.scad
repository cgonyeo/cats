$fn = 128;

inchesToMm = 25.40;

height = 0.3;

boltDiam = 0.5;
innerDiam = 2;

tubeDiam = 4.25;

scale([inchesToMm,inchesToMm,inchesToMm]) union() {
    difference() {
        cylinder(h=height, d=innerDiam);
        cylinder(h=height, d=boltDiam);
    }

    translate([0,4,0]) cylinder(h=height, d=tubeDiam);
    translate([-0.5,0.75,0]) cube(size=[1,1.5,height]);

    translate([0,6.5,0])
    linear_extrude(height = height)
    polygon(
        points = [
            [ -0.05, 0.05 ],
            [    0,   0 ],
            [  0.05, 0.05 ],
            [  0.5, -1  ],
            [ -0.5, -1  ],
        ]
    );
}

