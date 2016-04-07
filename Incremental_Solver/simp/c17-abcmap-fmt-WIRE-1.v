module	c17 (N1,N2,N3,N4,N5,N6,N7,N8,N9,N12,N13,N14);



input N1,N2,N3,N4,N5;

input D_0,D_1; //RE__ALLOW(00,11);
 
output N10,N11; // anything1

wire N6,N7,N8,N9,D_0_NOT,D_1_NOT,MUX_O_0,ED_0,ED_1,ED_2,ED_3,ED_4,ED_5,ED_6,ED_7,ED_8,ED_9; // anything 2;
     
nand2 gate1( .a(N1), .b(N3), .O(N6) );
nand2 gate2( .a(N3), .b(N4), .O(N8) );
nand2 gate3( .a(N2), .b(N8), .O(N7) );
nand2 gate4( .a(N8), .b(N5), .O(N9) );
nand2 gate5( .a(MUX_O_0), .b(N7), .O(N10) );
nand2 gate6( .a(N7), .b(N9), .O(N11) );
inv1 gate( .a(D_0),.O(D_0_NOT) );  // anything;
inv1 gate( .a(D_1),.O(D_1_NOT) );
and2 gate( .a(N1), .b(D_0_NOT), .O(ED_0) );
and2 gate( .a(N5), .b(D_0_NOT), .O(ED_1) );
and2 gate( .a(N4), .b(D_0), .O(ED_2) );
and2 gate( .a(N6), .b(D_0), .O(ED_3) );
and2 gate( .a(ED_0), .b(D_1_NOT), .O(ED_9) );
and2 gate( .a(ED_1), .b(D_1), .O(ED_7) );
and2 gate( .a(ED_2), .b(D_1_NOT), .O(ED_5) );
and2 gate( .a(ED_3), .b(D_1), .O(ED_4) );
or2  gate( .a(ED_4), .b(ED_5), .O(ED_6) );
or2  gate( .a(ED_6), .b(ED_7), .O(ED_8) );
or2  gate( .a(ED_9), .b(ED_8), .O(MUX_O_0) );

endmodule
