004001f0 addiu $29 $29 -88
004001f8 sw $30 80 $29 
00400200 addu $30 $0 $29
00400208 lui $2 4096
00400210 addiu $2 $2 944
00400218 sw $2 68 $30 
00400220 addiu $2 $0 7
00400228 sw $2 72 $30 
00400230 lw $2 72 $30 
00400238 bgez $2 00400248
00400240 j 00400bd0
00400248 lw $2 68 $30 
00400250 lw $4 68 $30 
00400258 addiu $3 $4 28
00400260 lw $2 0 $2 
00400268 lw $3 0 $3 
00400270 addu $2 $2 $3
00400278 sw $2 0 $30 
00400280 lw $2 68 $30 
00400288 lw $4 68 $30 
00400290 addiu $3 $4 28
00400298 lw $2 0 $2 
004002a0 lw $3 0 $3 
004002a8 subu $2 $2 $3
004002b0 sw $2 28 $30 
004002b8 lw $3 68 $30 
004002c0 addiu $2 $3 4
004002c8 lw $4 68 $30 
004002d0 addiu $3 $4 24
004002d8 lw $2 0 $2 
004002e0 lw $3 0 $3 
004002e8 addu $2 $2 $3
004002f0 sw $2 4 $30 
004002f8 lw $3 68 $30 
00400300 addiu $2 $3 4
00400308 lw $4 68 $30 
00400310 addiu $3 $4 24
00400318 lw $2 0 $2 
00400320 lw $3 0 $3 
00400328 subu $2 $2 $3
00400330 sw $2 24 $30 
00400338 lw $3 68 $30 
00400340 addiu $2 $3 8
00400348 lw $4 68 $30 
00400350 addiu $3 $4 20
00400358 lw $2 0 $2 
00400360 lw $3 0 $3 
00400368 addu $2 $2 $3
00400370 sw $2 8 $30 
00400378 lw $3 68 $30 
00400380 addiu $2 $3 8
00400388 lw $4 68 $30 
00400390 addiu $3 $4 20
00400398 lw $2 0 $2 
004003a0 lw $3 0 $3 
004003a8 subu $2 $2 $3
004003b0 sw $2 20 $30 
004003b8 lw $3 68 $30 
004003c0 addiu $2 $3 12
004003c8 lw $4 68 $30 
004003d0 addiu $3 $4 16
004003d8 lw $2 0 $2 
004003e0 lw $3 0 $3 
004003e8 addu $2 $2 $3
004003f0 sw $2 12 $30 
004003f8 lw $3 68 $30 
00400400 addiu $2 $3 12
00400408 lw $4 68 $30 
00400410 addiu $3 $4 16
00400418 lw $2 0 $2 
00400420 lw $3 0 $3 
00400428 subu $2 $2 $3
00400430 sw $2 16 $30 
00400438 lw $2 0 $30 
00400440 lw $3 12 $30 
00400448 addu $2 $2 $3
00400450 sw $2 32 $30 
00400458 lw $2 0 $30 
00400460 lw $3 12 $30 
00400468 subu $2 $2 $3
00400470 sw $2 44 $30 
00400478 lw $2 4 $30 
00400480 lw $3 8 $30 
00400488 addu $2 $2 $3
00400490 sw $2 36 $30 
00400498 lw $2 4 $30 
004004a0 lw $3 8 $30 
004004a8 subu $2 $2 $3
004004b0 sw $2 40 $30 
004004b8 lw $2 68 $30 
004004c0 lw $3 32 $30 
004004c8 lw $4 36 $30 
004004d0 addu $3 $3 $4
004004d8 sll $4 $3 0x2
004004e0 sw $4 0 $2 
004004e8 lw $3 68 $30 
004004f0 addiu $2 $3 16
004004f8 lw $3 32 $30 
00400500 lw $4 36 $30 
00400508 subu $3 $3 $4
00400510 sll $4 $3 0x2
00400518 sw $4 0 $2 
00400520 lw $2 40 $30 
00400528 lw $3 44 $30 
00400530 addu $2 $2 $3
00400538 addu $4 $0 $2
00400540 sll $3 $4 0x3
00400548 addu $3 $3 $2
00400550 sll $4 $3 0x4
00400558 subu $4 $4 $2
00400560 sll $2 $4 0x5
00400568 subu $3 $2 $4
00400570 sw $3 48 $30 
00400578 lw $3 68 $30 
00400580 addiu $2 $3 8
00400588 lw $3 44 $30 
00400590 addu $5 $0 $3
00400598 sll $4 $5 0x1
004005a0 addu $4 $4 $3
004005a8 sll $5 $4 0x4
004005b0 addu $5 $5 $3
004005b8 sll $4 $5 0x6
004005c0 subu $4 $4 $3
004005c8 sll $3 $4 0x1
004005d0 lw $4 48 $30 
004005d8 addu $3 $3 $4
004005e0 addiu $4 $3 1024
004005e8 sra $3 $4 0xb
004005f0 sw $3 0 $2 
004005f8 lw $3 68 $30 
00400600 addiu $2 $3 24
00400608 lw $3 40 $30 
00400610 addu $5 $0 $3
00400618 sll $4 $5 0x4
00400620 subu $4 $4 $3
00400628 sll $5 $4 0x2
00400630 subu $5 $5 $3
00400638 sll $4 $5 0x3
00400640 addu $4 $4 $3
00400648 sll $5 $4 0x5
00400650 addu $5 $5 $3
00400658 subu $3 $0 $5
00400660 lw $4 48 $30 
00400668 addu $3 $3 $4
00400670 addiu $4 $3 1024
00400678 sra $3 $4 0xb
00400680 sw $3 0 $2 
00400688 lw $2 16 $30 
00400690 lw $3 28 $30 
00400698 addu $2 $2 $3
004006a0 sw $2 48 $30 
004006a8 lw $2 20 $30 
004006b0 lw $3 24 $30 
004006b8 addu $2 $2 $3
004006c0 sw $2 52 $30 
004006c8 lw $2 16 $30 
004006d0 lw $3 24 $30 
004006d8 addu $2 $2 $3
004006e0 sw $2 56 $30 
004006e8 lw $2 20 $30 
004006f0 lw $3 28 $30 
004006f8 addu $2 $2 $3
00400700 sw $2 60 $30 
00400708 lw $2 56 $30 
00400710 lw $3 60 $30 
00400718 addu $2 $2 $3
00400720 addu $3 $0 $2
00400728 sll $4 $3 0x2
00400730 addu $4 $4 $2
00400738 sll $3 $4 0x4
00400740 subu $3 $3 $4
00400748 sll $4 $3 0x2
00400750 addu $4 $4 $2
00400758 sll $3 $4 0x5
00400760 addu $2 $3 $2
00400768 sw $2 64 $30 
00400770 lw $2 16 $30 
00400778 addu $4 $0 $2
00400780 sll $3 $4 0x3
00400788 addu $3 $3 $2
00400790 sll $4 $3 0x4
00400798 addu $3 $3 $4
004007a0 sll $4 $3 0x3
004007a8 subu $4 $4 $2
004007b0 sll $2 $4 0x1
004007b8 sw $2 16 $30 
004007c0 lw $2 20 $30 
004007c8 addu $4 $0 $2
004007d0 sll $3 $4 0x5
004007d8 addu $3 $3 $2
004007e0 sll $4 $3 0x3
004007e8 subu $4 $4 $2
004007f0 sll $3 $4 0x2
004007f8 subu $3 $3 $2
00400800 sll $4 $3 0x2
00400808 addu $4 $4 $2
00400810 sll $3 $4 0x2
00400818 subu $2 $3 $2
00400820 sw $2 20 $30 
00400828 lw $2 24 $30 
00400830 addu $4 $0 $2
00400838 sll $3 $4 0x1
00400840 addu $3 $3 $2
00400848 sll $4 $3 0x4
00400850 addu $3 $3 $4
00400858 sll $4 $3 0x2
00400860 subu $4 $4 $2
00400868 sll $2 $4 0x5
00400870 subu $2 $2 $4
00400878 sll $3 $2 0x2
00400880 sw $3 24 $30 
00400888 lw $2 28 $30 
00400890 addu $4 $0 $2
00400898 sll $3 $4 0x1
004008a0 addu $3 $3 $2
004008a8 sll $4 $3 0xa
004008b0 addu $3 $3 $4
004008b8 sll $4 $3 0x2
004008c0 subu $2 $4 $2
004008c8 sw $2 28 $30 
004008d0 lw $2 48 $30 
004008d8 addiu $3 $0 -7373
004008e0 mult $2 $3
004008e8 mflo $2
004008f0 sw $2 48 $30 
004008f8 lw $2 52 $30 
00400900 addu $4 $0 $2
00400908 sll $3 $4 0x2
00400910 addu $3 $3 $2
00400918 sll $4 $3 0x3
00400920 addu $4 $4 $2
00400928 sll $3 $4 0x7
00400930 addu $3 $3 $2
00400938 sll $4 $3 0x2
00400940 subu $2 $4 $2
00400948 subu $3 $0 $2
00400950 sw $3 52 $30 
00400958 lw $2 56 $30 
00400960 addu $4 $0 $2
00400968 sll $3 $4 0x6
00400970 subu $3 $3 $2
00400978 sll $4 $3 0x2
00400980 subu $4 $4 $2
00400988 sll $3 $4 0x4
00400990 addu $3 $3 $2
00400998 sll $4 $3 0x2
004009a0 addu $2 $4 $2
004009a8 subu $3 $0 $2
004009b0 sw $3 56 $30 
004009b8 lw $2 60 $30 
004009c0 addu $4 $0 $2
004009c8 sll $3 $4 0x1
004009d0 addu $3 $3 $2
004009d8 sll $4 $3 0x3
004009e0 addu $4 $4 $2
004009e8 sll $3 $4 0x5
004009f0 subu $3 $3 $2
004009f8 sll $2 $3 0x2
00400a00 subu $3 $0 $2
00400a08 sw $3 60 $30 
00400a10 lw $2 56 $30 
00400a18 lw $3 64 $30 
00400a20 addu $2 $2 $3
00400a28 sw $2 56 $30 
00400a30 lw $2 60 $30 
00400a38 lw $3 64 $30 
00400a40 addu $2 $2 $3
00400a48 sw $2 60 $30 
00400a50 lw $3 68 $30 
00400a58 addiu $2 $3 28
00400a60 lw $3 16 $30 
00400a68 lw $4 48 $30 
00400a70 addu $3 $3 $4
00400a78 lw $4 56 $30 
00400a80 addu $3 $3 $4
00400a88 addiu $4 $3 1024
00400a90 sra $3 $4 0xb
00400a98 sw $3 0 $2 
00400aa0 lw $3 68 $30 
00400aa8 addiu $2 $3 20
00400ab0 lw $3 20 $30 
00400ab8 lw $4 52 $30 
00400ac0 addu $3 $3 $4
00400ac8 lw $4 60 $30 
00400ad0 addu $3 $3 $4
00400ad8 addiu $4 $3 1024
00400ae0 sra $3 $4 0xb
00400ae8 sw $3 0 $2 
00400af0 lw $3 68 $30 
00400af8 addiu $2 $3 12
00400b00 lw $3 24 $30 
00400b08 lw $4 52 $30 
00400b10 addu $3 $3 $4
00400b18 lw $4 56 $30 
00400b20 addu $3 $3 $4
00400b28 addiu $4 $3 1024
00400b30 sra $3 $4 0xb
00400b38 sw $3 0 $2 
00400b40 lw $3 68 $30 
00400b48 addiu $2 $3 4
00400b50 lw $3 28 $30 
00400b58 lw $4 48 $30 
00400b60 addu $3 $3 $4
00400b68 lw $4 60 $30 
00400b70 addu $3 $3 $4
00400b78 addiu $4 $3 1024
00400b80 sra $3 $4 0xb
00400b88 sw $3 0 $2 
00400b90 lw $2 68 $30 
00400b98 addiu $3 $2 32
00400ba0 sw $3 68 $30 
00400ba8 lw $3 72 $30 
00400bb0 addiu $2 $3 -1
00400bb8 addu $3 $0 $2
00400bc0 sw $3 72 $30 
00400bc8 j 00400230
00400bd0 lui $2 4096
00400bd8 addiu $2 $2 944
00400be0 sw $2 68 $30 
00400be8 addiu $2 $0 7
00400bf0 sw $2 72 $30 
00400bf8 lw $2 72 $30 
00400c00 bgez $2 00400c10
00400c08 j 004015b0
00400c10 lw $2 68 $30 
00400c18 lw $4 68 $30 
00400c20 addiu $3 $4 224
00400c28 lw $2 0 $2 
00400c30 lw $3 0 $3 
00400c38 addu $2 $2 $3
00400c40 sw $2 0 $30 
00400c48 lw $2 68 $30 
00400c50 lw $4 68 $30 
00400c58 addiu $3 $4 224
00400c60 lw $2 0 $2 
00400c68 lw $3 0 $3 
00400c70 subu $2 $2 $3
00400c78 sw $2 28 $30 
00400c80 lw $3 68 $30 
00400c88 addiu $2 $3 32
00400c90 lw $4 68 $30 
00400c98 addiu $3 $4 192
00400ca0 lw $2 0 $2 
00400ca8 lw $3 0 $3 
00400cb0 addu $2 $2 $3
00400cb8 sw $2 4 $30 
00400cc0 lw $3 68 $30 
00400cc8 addiu $2 $3 32
00400cd0 lw $4 68 $30 
00400cd8 addiu $3 $4 192
00400ce0 lw $2 0 $2 
00400ce8 lw $3 0 $3 
00400cf0 subu $2 $2 $3
00400cf8 sw $2 24 $30 
00400d00 lw $3 68 $30 
00400d08 addiu $2 $3 64
00400d10 lw $4 68 $30 
00400d18 addiu $3 $4 160
00400d20 lw $2 0 $2 
00400d28 lw $3 0 $3 
00400d30 addu $2 $2 $3
00400d38 sw $2 8 $30 
00400d40 lw $3 68 $30 
00400d48 addiu $2 $3 64
00400d50 lw $4 68 $30 
00400d58 addiu $3 $4 160
00400d60 lw $2 0 $2 
00400d68 lw $3 0 $3 
00400d70 subu $2 $2 $3
00400d78 sw $2 20 $30 
00400d80 lw $3 68 $30 
00400d88 addiu $2 $3 96
00400d90 lw $4 68 $30 
00400d98 addiu $3 $4 128
00400da0 lw $2 0 $2 
00400da8 lw $3 0 $3 
00400db0 addu $2 $2 $3
00400db8 sw $2 12 $30 
00400dc0 lw $3 68 $30 
00400dc8 addiu $2 $3 96
00400dd0 lw $4 68 $30 
00400dd8 addiu $3 $4 128
00400de0 lw $2 0 $2 
00400de8 lw $3 0 $3 
00400df0 subu $2 $2 $3
00400df8 sw $2 16 $30 
00400e00 lw $2 0 $30 
00400e08 lw $3 12 $30 
00400e10 addu $2 $2 $3
00400e18 sw $2 32 $30 
00400e20 lw $2 0 $30 
00400e28 lw $3 12 $30 
00400e30 subu $2 $2 $3
00400e38 sw $2 44 $30 
00400e40 lw $2 4 $30 
00400e48 lw $3 8 $30 
00400e50 addu $2 $2 $3
00400e58 sw $2 36 $30 
00400e60 lw $2 4 $30 
00400e68 lw $3 8 $30 
00400e70 subu $2 $2 $3
00400e78 sw $2 40 $30 
00400e80 lw $2 68 $30 
00400e88 lw $3 32 $30 
00400e90 lw $4 36 $30 
00400e98 addu $3 $3 $4
00400ea0 addiu $4 $3 2
00400ea8 sra $3 $4 0x2
00400eb0 sw $3 0 $2 
00400eb8 lw $3 68 $30 
00400ec0 addiu $2 $3 128
00400ec8 lw $3 32 $30 
00400ed0 lw $4 36 $30 
00400ed8 subu $3 $3 $4
00400ee0 addiu $4 $3 2
00400ee8 sra $3 $4 0x2
00400ef0 sw $3 0 $2 
00400ef8 lw $2 40 $30 
00400f00 lw $3 44 $30 
00400f08 addu $2 $2 $3
00400f10 addu $4 $0 $2
00400f18 sll $3 $4 0x3
00400f20 addu $3 $3 $2
00400f28 sll $4 $3 0x4
00400f30 subu $4 $4 $2
00400f38 sll $2 $4 0x5
00400f40 subu $3 $2 $4
00400f48 sw $3 48 $30 
00400f50 lw $3 68 $30 
00400f58 addiu $2 $3 64
00400f60 lw $3 44 $30 
00400f68 addu $5 $0 $3
00400f70 sll $4 $5 0x1
00400f78 addu $4 $4 $3
00400f80 sll $5 $4 0x4
00400f88 addu $5 $5 $3
00400f90 sll $4 $5 0x6
00400f98 subu $4 $4 $3
00400fa0 sll $3 $4 0x1
00400fa8 lw $4 48 $30 
00400fb0 addu $3 $3 $4
00400fb8 addiu $4 $3 16384
00400fc0 sra $3 $4 0xf
00400fc8 sw $3 0 $2 
00400fd0 lw $3 68 $30 
00400fd8 addiu $2 $3 192
00400fe0 lw $3 40 $30 
00400fe8 addu $5 $0 $3
00400ff0 sll $4 $5 0x4
00400ff8 subu $4 $4 $3
00401000 sll $5 $4 0x2
00401008 subu $5 $5 $3
00401010 sll $4 $5 0x3
00401018 addu $4 $4 $3
00401020 sll $5 $4 0x5
00401028 addu $5 $5 $3
00401030 subu $3 $0 $5
00401038 lw $4 48 $30 
00401040 addu $3 $3 $4
00401048 addiu $4 $3 16384
00401050 sra $3 $4 0xf
00401058 sw $3 0 $2 
00401060 lw $2 16 $30 
00401068 lw $3 28 $30 
00401070 addu $2 $2 $3
00401078 sw $2 48 $30 
00401080 lw $2 20 $30 
00401088 lw $3 24 $30 
00401090 addu $2 $2 $3
00401098 sw $2 52 $30 
004010a0 lw $2 16 $30 
004010a8 lw $3 24 $30 
004010b0 addu $2 $2 $3
004010b8 sw $2 56 $30 
004010c0 lw $2 20 $30 
004010c8 lw $3 28 $30 
004010d0 addu $2 $2 $3
004010d8 sw $2 60 $30 
004010e0 lw $2 56 $30 
004010e8 lw $3 60 $30 
004010f0 addu $2 $2 $3
004010f8 addu $3 $0 $2
00401100 sll $4 $3 0x2
00401108 addu $4 $4 $2
00401110 sll $3 $4 0x4
00401118 subu $3 $3 $4
00401120 sll $4 $3 0x2
00401128 addu $4 $4 $2
00401130 sll $3 $4 0x5
00401138 addu $2 $3 $2
00401140 sw $2 64 $30 
00401148 lw $2 16 $30 
00401150 addu $4 $0 $2
00401158 sll $3 $4 0x3
00401160 addu $3 $3 $2
00401168 sll $4 $3 0x4
00401170 addu $3 $3 $4
00401178 sll $4 $3 0x3
00401180 subu $4 $4 $2
00401188 sll $2 $4 0x1
00401190 sw $2 16 $30 
00401198 lw $2 20 $30 
004011a0 addu $4 $0 $2
004011a8 sll $3 $4 0x5
004011b0 addu $3 $3 $2
004011b8 sll $4 $3 0x3
004011c0 subu $4 $4 $2
004011c8 sll $3 $4 0x2
004011d0 subu $3 $3 $2
004011d8 sll $4 $3 0x2
004011e0 addu $4 $4 $2
004011e8 sll $3 $4 0x2
004011f0 subu $2 $3 $2
004011f8 sw $2 20 $30 
00401200 lw $2 24 $30 
00401208 addu $4 $0 $2
00401210 sll $3 $4 0x1
00401218 addu $3 $3 $2
00401220 sll $4 $3 0x4
00401228 addu $3 $3 $4
00401230 sll $4 $3 0x2
00401238 subu $4 $4 $2
00401240 sll $2 $4 0x5
00401248 subu $2 $2 $4
00401250 sll $3 $2 0x2
00401258 sw $3 24 $30 
00401260 lw $2 28 $30 
00401268 addu $4 $0 $2
00401270 sll $3 $4 0x1
00401278 addu $3 $3 $2
00401280 sll $4 $3 0xa
00401288 addu $3 $3 $4
00401290 sll $4 $3 0x2
00401298 subu $2 $4 $2
004012a0 sw $2 28 $30 
004012a8 lw $2 48 $30 
004012b0 addiu $3 $0 -7373
004012b8 mult $2 $3
004012c0 mflo $2
004012c8 sw $2 48 $30 
004012d0 lw $2 52 $30 
004012d8 addu $4 $0 $2
004012e0 sll $3 $4 0x2
004012e8 addu $3 $3 $2
004012f0 sll $4 $3 0x3
004012f8 addu $4 $4 $2
00401300 sll $3 $4 0x7
00401308 addu $3 $3 $2
00401310 sll $4 $3 0x2
00401318 subu $2 $4 $2
00401320 subu $3 $0 $2
00401328 sw $3 52 $30 
00401330 lw $2 56 $30 
00401338 addu $4 $0 $2
00401340 sll $3 $4 0x6
00401348 subu $3 $3 $2
00401350 sll $4 $3 0x2
00401358 subu $4 $4 $2
00401360 sll $3 $4 0x4
00401368 addu $3 $3 $2
00401370 sll $4 $3 0x2
00401378 addu $2 $4 $2
00401380 subu $3 $0 $2
00401388 sw $3 56 $30 
00401390 lw $2 60 $30 
00401398 addu $4 $0 $2
004013a0 sll $3 $4 0x1
004013a8 addu $3 $3 $2
004013b0 sll $4 $3 0x3
004013b8 addu $4 $4 $2
004013c0 sll $3 $4 0x5
004013c8 subu $3 $3 $2
004013d0 sll $2 $3 0x2
004013d8 subu $3 $0 $2
004013e0 sw $3 60 $30 
004013e8 lw $2 56 $30 
004013f0 lw $3 64 $30 
004013f8 addu $2 $2 $3
00401400 sw $2 56 $30 
00401408 lw $2 60 $30 
00401410 lw $3 64 $30 
00401418 addu $2 $2 $3
00401420 sw $2 60 $30 
00401428 lw $3 68 $30 
00401430 addiu $2 $3 224
00401438 lw $3 16 $30 
00401440 lw $4 48 $30 
00401448 addu $3 $3 $4
00401450 lw $4 56 $30 
00401458 addu $3 $3 $4
00401460 addiu $4 $3 16384
00401468 sra $3 $4 0xf
00401470 sw $3 0 $2 
00401478 lw $3 68 $30 
00401480 addiu $2 $3 160
00401488 lw $3 20 $30 
00401490 lw $4 52 $30 
00401498 addu $3 $3 $4
004014a0 lw $4 60 $30 
004014a8 addu $3 $3 $4
004014b0 addiu $4 $3 16384
004014b8 sra $3 $4 0xf
004014c0 sw $3 0 $2 
004014c8 lw $3 68 $30 
004014d0 addiu $2 $3 96
004014d8 lw $3 24 $30 
004014e0 lw $4 52 $30 
004014e8 addu $3 $3 $4
004014f0 lw $4 56 $30 
004014f8 addu $3 $3 $4
00401500 addiu $4 $3 16384
00401508 sra $3 $4 0xf
00401510 sw $3 0 $2 
00401518 lw $3 68 $30 
00401520 addiu $2 $3 32
00401528 lw $3 28 $30 
00401530 lw $4 48 $30 
00401538 addu $3 $3 $4
00401540 lw $4 60 $30 
00401548 addu $3 $3 $4
00401550 addiu $4 $3 16384
00401558 sra $3 $4 0xf
00401560 sw $3 0 $2 
00401568 lw $3 68 $30 
00401570 addiu $2 $3 4
00401578 addu $3 $0 $2
00401580 sw $3 68 $30 
00401588 lw $3 72 $30 
00401590 addiu $2 $3 -1
00401598 addu $3 $0 $2
004015a0 sw $3 72 $30 
004015a8 j 00400bf8
004015b0 addu $29 $0 $30
004015b8 lw $30 80 $29 
004015c0 addiu $29 $29 88
004015c8 jr $31
004015d0 addiu $29 $29 -32
004015d8 sw $31 28 $29 
004015e0 sw $30 24 $29 
004015e8 addu $30 $0 $29
004015f0 addiu $2 $0 1
004015f8 sw $2 20 $30 
00401600 sw $0 16 $30 
00401608 lw $2 16 $30 
00401610 slti $3 $2 64
00401618 bne $3 $0 00401628
00401620 j 00401748
00401628 lw $2 20 $30 
00401630 addu $4 $0 $2
00401638 sll $3 $4 0x5
00401640 addu $3 $3 $2
00401648 sll $4 $3 0x2
00401650 addu $4 $4 $2
00401658 addiu $2 $4 81
00401660 lui $6 32768
00401668 ori $6 $6 32769
00401670 mult $2 $6
00401678 mfhi $5
00401680 mflo $4
00401688 srl $6 $5 0x0
00401690 addu $7 $0 $0
00401698 addu $3 $2 $6
004016a0 sra $4 $3 0xf
004016a8 sra $5 $2 0x1f
004016b0 subu $3 $4 $5
004016b8 addu $5 $0 $3
004016c0 sll $4 $5 0x10
004016c8 subu $4 $4 $3
004016d0 subu $2 $2 $4
004016d8 sw $2 20 $30 
004016e0 lw $2 16 $30 
004016e8 addu $3 $0 $2
004016f0 sll $2 $3 0x2
004016f8 lui $3 4096
00401700 addiu $3 $3 944
00401708 addu $2 $2 $3
00401710 lw $3 20 $30 
00401718 sw $3 0 $2 
00401720 lw $3 16 $30 
00401728 addiu $2 $3 1
00401730 addu $3 $0 $2
00401738 sw $3 16 $30 
00401740 j 00401608
00401748 jal 004001f0
00401750 addu $29 $0 $30
00401758 lw $31 28 $29 
00401760 lw $30 24 $29 
00401768 addiu $29 $29 32
00401770 jr $31
00401778 nop 