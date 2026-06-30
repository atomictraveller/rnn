#pragma once
#include "resource.h"

/* // menu flags for resource.h
#define IDM_NEWDATA				112
#define IDM_OPENDATA			113
#define IDM_SAVEDATA			114
#define IDM_TRAIN				115
#define IDM_STOPTRAIN			116
#define IDM_TRAIN10X			117
#define IDM_USEGRADNORM			118
#define IDM_USEADAM				119
#define IDM_OUTPUT				120
// for your menu branch in your .rc file
	BEGIN
		MENUITEM "N&ew data",			IDM_NEWDATA
		MENUITEM "O&pen data",			IDM_OPENDATA
		MENUITEM "S&ave data",			IDM_SAVEDATA
		MENUITEM "T&rain",				IDM_TRAIN
		MENUITEM "Stop training",		IDM_STOPTRAIN
		MENUITEM "10 trains",			IDM_TRAIN10X
		MENUITEM "use gradient norm.",	IDM_USEGRADNORM, CHECKED
		MENUITEM "use ADAM",			IDM_USEADAM, CHECKED
		MENUITEM "Output",				IDM_OUTPUT
	END
*/

HDC hdcframe;
HBITMAP oldframe;
HBITMAP framebmp;
int* framebuf;

HDC hdcbackground;		HBITMAP oldbackground;	HBITMAP backgroundbmp;	int* backgroundbuf;
HWND hEdit = NULL;

int dispx = 1026;
int dispy = 594;
HFONT hfont0, hfont1;
short int timerate = 32;	//	rate for gui timer

bool showinput = 0;	//	show input field
char intext[512] = { 0 };
char outext[8][128] = { 0 };
unsigned long int rnd;
char stop = 0;	//	stop training flag

#define idim 96		//	input/output layer size
#define hidd 256	//	hidden layer size
#define wind 64		//	truncated BPTT depth, written for ^2 values
#define winp (wind + 1)
#define winm (wind - 1)

char b = 32; //  previous c
char c;

//	hidden1[t] = tanh(W_xh1 * input[t] + W_h1h1 * hidden1[t - 1] + b1);
//	hidden2[t] = tanh(W_h1h2 * hidden1[t] + W_h2h2 * hidden2[t - 1] + b2);
//	output[t] = W_h2y * hidden2[t] + b_y;

//	elman style rnn

float nni[idim][hidd];		//  weights input layer to h0
float nnh0[hidd][hidd];		//  weights h0 to previous h0
float nnh0h[hidd][hidd];	//	weights h0 to h (configured [h][h0])
float nnh[hidd][hidd];		//  weights hidden to previous hidden
float nno[hidd][idim];		//	weights output layer

float h0[winp][hidd];		//  hidden layer 1 - uses fancy extra place scheme for forward and backward buffering
float h[winp][hidd];		//  hidden layer 2 - as above
float h0bias[hidd];			//	bias
float hbias[hidd];
float obias[idim];

float dnni[idim][hidd];   //  differences for backpropogation
float dnnh0[hidd][hidd];
float dnnh0h[hidd][hidd];
float dnnh[hidd][hidd]; //  ms compiler complains about size of stack when local
float dnno[hidd][idim];   //  so these three local variables are here instead

float netin[idim];		//  "hot one" array to input char - not actually used...
float netout[wind][idim];	//	outout/prediction

float softsum[wind];
float dh0bias[hidd];
float dhbias[hidd];
float dobias[idim];
float dh0[hidd];	//	temp. array used during back prop
float dh0_raw[hidd];
float dh0_next[hidd];
float dh[hidd];	//	temp. array used during back prop
float dh_raw[hidd];
float dh_next[hidd];
char cin[wind];	//	input char per iteration

bool GRADNORM = 1;	//	use gradient normalisation
bool ADAM = 0;	//	use ADAM optimisation
int adamt = 0;
const float beta1 = 0.9f;
const float beta2 = 0.999f;
const float beta1m = 0.1f;
const float beta2m = 0.001f;
const float epsilon = 1e-8f;

float mnni[idim][hidd] = { 0 };
float mnnh0[hidd][hidd] = { 0 };
float mnnh0h[hidd][hidd] = { 0 };
float mnnh[hidd][hidd] = { 0 };
float mnno[hidd][idim] = { 0 };
float mh0bias[hidd] = { 0 };
float mhbias[hidd] = { 0 };
float mobias[idim] = { 0 };

float vnni[idim][hidd] = { 0 };
float vnnh0[hidd][hidd] = { 0 };
float vnnh0h[hidd][hidd] = { 0 };
float vnnh[hidd][hidd] = { 0 };
float vnno[hidd][idim] = { 0 };
float vh0bias[hidd] = { 0 };
float vhbias[hidd] = { 0 };
float vobias[idim] = { 0 };



unsigned long int tests = 0;
int thismany = 0;	//	setup for 10 training sessions
bool do10x = 0;

float learn = .01f;
float temp = 1.f;   //  temperature/confidence.. low values = higher repeats/confidence
float ebuf = 0.f;   //  error accumulator
float edisp = 0.f;   //  averaged display coefficient (e^edisp = # chars not sure)

char dispix = 0;	//	for stepping throigh colours :)


//float learnarr[37] = { .1f,.09f,.08f,.07f,.06f,.05f,.04f,.03f,.02f,.01f,.009f,.008f,.007f,.006f,.005f,.004f,.003f,.002f,.001f,.0009f,.0008f,.0007f,.0006f,.0005f,.0004f,.0003f,.0002f,.0001f,.00009f,.00008f,.00007f,.00006f,.00005f,.00004f,.00003f,.00002f,.00001f };
float learnarr[34] = {/*was28  .56234f,.31622f,.17782f,.1f,
	.056234f,.031622f,.017782f,.01f,
	5.6234e-3f, 3.1622e-3f, 1.7782e-3f, 1e-3f,
	5.6234e-4f, 3.1622e-4f, 1.7782e-4f, 1e-4f,
	5.6234e-5f, 3.1622e-5f, 1.7782e-5f, 1e-5f,
	5.6234e-6f, 3.1622e-6f, 1.7782e-6f, 1e-6f,
	5.6234e-7f, 3.1622e-7f, 1.7782e-7f, 1e-7f,*/
	0.5f,
	0.25f,
	0.125f, 0.1f,
	0.0625f,
	0.03125f,
	0.015625f, 0.01f,
	0.0078125f,
	0.00390625f,

	0.001953125f, 0.001f,
	0.0009765625f,
	0.00048828125f,
	0.000244140625f,
	0.0001220703125f, 0.0001f,
	0.00006103515625f,
	0.000030517578125f,
	0.0000152587890625f, 0.00001f,

	0.00000762939453125f,
	0.000003814697265625f,
	0.0000019073486328125f, 0.000001f,
	0.00000095367431640625f,
	0.000000476837158203125f,
	0.0000002384185791015625f,
	0.00000011920928955078125f, 0.0000001f,
	0.000000059604644775390625f,

	0.0000000298023223876953125f,
	0.00000001490116119384765625f, 0.00000001f
};
float regarr[31] = { 1e-4f, 3e-5f, 1e-5f, 3e-6f, 1e-6f, 3e-7f, 1e-7f, 3e-8f, 1e-8f, 3e-9f, 1e-9f,
1e-10f, 3e-11f, 1e-11f, 3e-12f, 1e-12f, 3e-13f, 1e-13f, 3e-14f, 1e-14f, 3e-15f, 1e-15f, 3e-16f, 1e-16f, 3e-17f, 1e-17f, 3e-18f, 1e-18f, 3e-19f, 1e-19f, 0.f };
float temparr[10] = { 1.f, .9f, .8f, .7f, .6f, .5f, .4f, .3f, .2f, .1f };
int learnix = 7;    //11  indexes for incrementing parameter value
int regix = 30;	//	was 19 of 20
int tempix = 0;

char out1[128];     //  gui display string
char out0[128];     //  target string

char displayindex = 0;




/*
char chargroup[126] = {	//	encoding one char in 2 parameters didn't seem to work real well.. 
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,
	0,1,1,2,2,2,2,1,	1,1,2,2,1,2,1,2,	4,4,4,4,4,4,4,4,	4,4,1,1,2,2,2,1,
	3,8,10,10,10,8,9,10,9,8,10,10,9,9,9,8,	10,10,9,9,10,8,9,9,	10,9,10,3,3,3,3,3,
	3,5,7,7,7,5,6,7,	6,5,7,7,6,6,6,5,	7,7,6,6,7,5,6,6,	7,6,7,3,3,3 };

char charindex[126] = {
	0,0,0,0,0,0,0,0,	0,0,1,0,0,2,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,
	3,0,1,0,1,2,3,2,	3,4,4,5,5,6,6,7,	0,1,2,3,4,5,6,7,	8,9,7,8,8,9,10,9,
	0,0,0,1,2,1,0,3,	1,2,4,5,2,3,4,3,	6,7,5,6,8,4,7,8,	9,9,10,1,2,3,4,5,
	6,0,0,1,2,1,0,3,	1,2,4,5,2,3,4,3,	6,7,5,6,8,4,7,8,	9,9,10,7,8,9 };

char decodechar[11][11] = {
	  0, 10, 13, 32,  0,  0,  0,  0,  0,  0,  0,
	 33, 34, 39, 40, 41, 44, 46, 58, 59, 63,  0,
	 35, 36, 37, 38, 42, 43, 45, 47, 60, 61, 62,
	 64, 91, 92, 93, 94, 95, 96,123,124,125,  0,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,  0,
	 97,101,105,111,117,  0,  0,  0,  0,  0,  0,
	102,104,108,109,110,114,115,118,119,121,  0,
	 98, 99,100,103,106,107,112,113,116,120,122,
	 65, 69, 73, 79, 85,  0,  0,  0,  0,  0,  0,
	 70, 72, 76, 77, 78, 82, 83, 86, 87, 89,  0,
	 66, 67, 68, 71, 74, 75, 80, 81, 84, 88, 90
};

group	(group and index are scaled * 0.1 and expanded * 10)
0	formatting
1	punctuation
2	symbols
3	symbols 2
4	numerals
5	lowercase vowels			aeiou
6	lowercase soft consonants	fhlmnrsvwy
7	lowercase hard consonants	bcdgjkpqtxz
8	capital vowels
9	capital soft consonants
10	capital hard consonants
 

regularisation "10-4 to 10-2 prevents overfitting.." L2 style regularization here.. ;) changes large error response
temperature = confidence or enthusiasm for learning the subject..
ADAM.. helps not get stuck in local minima

input and output use "one hot" arrays.. if you have 128 characters, you have 128 nodes, one is 1, the rest are 0
this seems to be a common way of formatting text information for RNN, using dynamic arrays and determining
the number of unique elements in your data, often applied at the word level rather than character.

using [enter] in .txt file results in 13, 10 (0d, 0a)
in practice EOF is byte 0 not 03 or anything else
.txt is created with no header on win10, first bytes are first CHAR
  0  NUL (null)                      32  SPACE     64  @         96  `
  1  SOH (start of heading)          33  !         65  A         97  a
  2  STX (start of text)             34  "         66  B         98  b
  3  ETX (end of text)               35  #         67  C         99  c
  4  EOT (end of transmission)       36  $         68  D        100  d
  5  ENQ (enquiry)                   37  %         69  E        101  e
  6  ACK (acknowledge)               38  &         70  F        102  f
  7  BEL (bell)                      39  '         71  G        103  g
  8  BS  (backspace)                 40  (         72  H        104  h
  9  TAB (horizontal tab)            41  )         73  I        105  i
 10  LF  (NL line feed, new line)    42  *         74  J        106  j
 11  VT  (vertical tab)              43  +         75  K        107  k
 12  FF  (NP form feed, new page)    44  ,         76  L        108  l
 13  CR  (carriage return)           45  -         77  M        109  m
 14  SO  (shift out)                 46  .         78  N        110  n
 15  SI  (shift in)                  47  /         79  O        111  o
 16  DLE (data link escape)          48  0         80  P        112  p
 17  DC1 (device control 1)          49  1         81  Q        113  q
 18  DC2 (device control 2)          50  2         82  R        114  r
 19  DC3 (device control 3)          51  3         83  S        115  s
 20  DC4 (device control 4)          52  4         84  T        116  t
 21  NAK (negative acknowledge)      53  5         85  U        117  u
 22  SYN (synchronous idle)          54  6         86  V        118  v
 23  ETB (end of trans. block)       55  7         87  W        119  w
 24  CAN (cancel)                    56  8         88  X        120  x
 25  EM  (end of medium)             57  9         89  Y        121  y
 26  SUB (substitute)                58  :         90  Z        122  z
 27  ESC (escape)                    59  ;         91  [        123  {
 28  FS  (file separator)            60  <         92  \        124  |
 29  GS  (group separator)           61  =         93  ]        125  }
 30  RS  (record separator)          62  >         94  ^        126  ~
 31  US  (unit separator)            63  ?         95  _        127  DEL

formatting	3	10, 13, 32 (lf, cr, space)
symbols		31	33-47	58-64	91-96	123-125
numerals	10	48-57 0-9
capitals	26	65-90
lowercase	26	97-122

				G	Index
				0	0	null
10		LF		0	1
13		CR		0	2
32		SPACE	0	3
33-34	!"		1	0,1
35-38	#$%&	2	0-3
39-41	`()		1	2-4
42-43	*+		2	4,5
44		,		1	5
45		-		2	6
46		.		1	6
47		/		2	7
48-57	0-9		4	0-9

58-59	:;		1	7,8
60-62	<=>		2	8-10
63		?		1	9
64		@		3	0

91-96	[\]^_`	3	1-6

123-125	{|}		3	7-9



*/