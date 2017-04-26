# Based on Ken Perlins java code for improved noise.
# See http://mrl.nyu.edu/~perlin/noise/ for more details
# Malcolm Kesson Oct 2006

set permutations { 	151 160 137 91 90 15 131 13 201 95 96 53 194 233 7 225 140 36 103 
					30 69 142 8 99 37 240 21 10 23 190 6 148 247 120 234 75 0 26 197 
					62 94 252 219 203 117 35 11 32 57 177 33 88 237 149 56 87 174 20 
					125 136 171 168 68 175 74 165 71 134 139 48 27 166 77 146 158 231 
					83 111 229 122 60 211 133 230 220 105 92 41 55 46 245 40 244 102 
					143 54 65 25 63 161 1 216 80 73 209 76 132 187 208 89 18 169 200 
					196 135 130 116 188 159 86 164 100 109 198 173 186 3 64 52 217 226 
					250 124 123 5 202 38 147 118 126 255 82 85 212 207 206 59 227 47 16 
					58 17 182 189 28 42 223 183 170 213 119 248 152 2 44 154 163 70 
					221 153 101 155 167 43 172 9 129 22 39 253 19 98 108 110 79 113 
					224 232 178 185 112 104 218 246 97 228 251 34 242 193 238 210 144 
					12 191 179 162 241 81 51 145 235 249 14 239 107 49 192 214 31 181 
					199 106 157 184 84 204 176 115 121 50 45 127 4 150 254 138 236 
					205 93 222 114 67 29 24 72 243 141 128 195 78 66 215 61 156 180 }

proc lerp { t a b } {
	return [expr $a + $t * ($b - $a)]
	}

proc fade { t } {
	return [expr $t * $t * $t * ($t * ($t * 6 - 15) + 10)]
	}

proc grad { hash x y z } {
	set h [expr $hash & 15]
	set u [expr $h < 8 ? $x : $y]
	set v [expr $h < 4 ? $y : $h == 12 || $h == 14 ? $x : $z]
	return [expr (($h & 1) == 0 ? $u : -$u) + (($h & 2) == 0 ? $v : -$v)]
	}

#============================================================
proc pnoise { x y z } {
	global permutations
	set p ""
	append p $permutations
	append p " "
	append p $permutations
	
	set X [expr int(floor($x)) & 255]
	set Y [expr int(floor($y)) & 255]
	set Z [expr int(floor($z)) & 255]
	
	set x [expr $x - floor($x)]
	set y [expr $y - floor($y)]
	set z [expr $z - floor($z)]
	
	set u [fade $x]
	set v [fade $y]
	set w [fade $z]
	
	set A  [expr [lindex $p $X] + $Y]
	set AA [expr [lindex $p $A] + $Z]
	set AB [expr [lindex $p [expr $A + 1] ] + $Z]
	
	set B  [expr [lindex $p [expr $X + 1] ] + $Y]
	set BA [expr [lindex $p $B] + $Z]
	set BB [expr [lindex $p [expr $B + 1] ] + $Z]

	set pAA  [lindex $p $AA]
	set pAB  [lindex $p $AB]
	set pBA  [lindex $p $BA]
	set pBB  [lindex $p $BB]
	set pAA1 [lindex $p [expr $AA + 1]]
	set pBA1 [lindex $p [expr $BA + 1]]
	set pAB1 [lindex $p [expr $AB + 1]]
	set pBB1 [lindex $p [expr $BB + 1]]
	
	set gradAA  [grad $pAA $x $y $z]
	set gradBA  [grad $pBA [expr $x - 1] $y $z]
	set gradAB  [grad $pAB $x [expr $y - 1] $z]
	set gradBB  [grad $pBB [expr $x - 1] [expr $y - 1] $z]
	set gradAA1 [grad $pAA1 $x $y [expr $z - 1]]
	set gradBA1 [grad $pBA1 [expr $x - 1] $y [expr $z - 1]]
	set gradAB1 [grad $pAB1 $x [expr $y - 1] [expr $z - 1]]
	set gradBB1 [grad $pBB1 [expr $x - 1] [expr $y - 1] [expr $z - 1]] 
	
	return [lerp $w [lerp $v [lerp $u $gradAA $gradBA] [lerp $u $gradAB $gradBB]]  [lerp $v [lerp $u $gradAA1 $gradBA1] [lerp $u $gradAB1 $gradBB1]]]
	}
