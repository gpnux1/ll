#include "gba/types.h"
#include "script_vm.h"

/* ==========================================================================
 * 脚本与音效资源综合指针总表 (0x087ED6D4, 363 项)
 *   - 索引 0..142   : 剧情/地图事件脚本集 (LZ77 压缩数据块, 详见 data/script_data.s)
 *                     例如 SetId 1 = gScriptSet_001 (NewGame_Init Burg 村开场剧情)
 *   - 索引 143..362 : 歌曲/音效头指针表 (M4A Song Headers, 详见 sound/songs/)
 * ========================================================================== */
const u32 gScriptSetTable[363] = {
    0x0862D8A4,  /* [  0] -> gScriptSet_000 */
    0x0862E2A0,  /* [  1] -> gScriptSet_001 (NewGame_Init: ScriptSet_Load(1,0,1) Burg 村开场) */
    0x08631CE8,  /* [  2] -> gScriptSet_002 */
    0x08632B38,  /* [  3] -> gScriptSet_003 */
    0x08634F20,  /* [  4] -> gScriptSet_004 */
    0x0862D858,  /* [  5] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [  6] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [  7] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [  8] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [  9] -> gScriptSet_Empty (空脚本集) */
    0x08638C6C,  /* [ 10] -> gScriptSet_010 */
    0x0862D858,  /* [ 11] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 12] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 13] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 14] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 15] -> gScriptSet_Empty (空脚本集) */
    0x0863D240,  /* [ 16] -> gScriptSet_016 */
    0x0863DAAC,  /* [ 17] -> gScriptSet_017 */
    0x0863E5A4,  /* [ 18] -> gScriptSet_018 */
    0x086413B0,  /* [ 19] -> gScriptSet_019 */
    0x08642A58,  /* [ 20] -> gScriptSet_020 */
    0x08644D6C,  /* [ 21] -> gScriptSet_021 */
    0x08648030,  /* [ 22] -> gScriptSet_022 */
    0x0862D858,  /* [ 23] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 24] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 25] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 26] -> gScriptSet_Empty (空脚本集) */
    0x08649F44,  /* [ 27] -> gScriptSet_027 */
    0x0864C404,  /* [ 28] -> gScriptSet_028 */
    0x0862D858,  /* [ 29] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 30] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 31] -> gScriptSet_Empty (空脚本集) */
    0x08650810,  /* [ 32] -> gScriptSet_032 */
    0x08650E60,  /* [ 33] -> gScriptSet_033 */
    0x086514F4,  /* [ 34] -> gScriptSet_034 */
    0x08653D1C,  /* [ 35] -> gScriptSet_035 */
    0x086542BC,  /* [ 36] -> gScriptSet_036 */
    0x08654B08,  /* [ 37] -> gScriptSet_037 */
    0x086578DC,  /* [ 38] -> gScriptSet_038 */
    0x08658D7C,  /* [ 39] -> gScriptSet_039 */
    0x0862D858,  /* [ 40] -> gScriptSet_Empty (空脚本集) */
    0x0865C2C0,  /* [ 41] -> gScriptSet_041 */
    0x0865CBE4,  /* [ 42] -> gScriptSet_042 */
    0x0865E964,  /* [ 43] -> gScriptSet_043 */
    0x08660004,  /* [ 44] -> gScriptSet_044 */
    0x08660084,  /* [ 45] -> gScriptSet_045 */
    0x086601A0,  /* [ 46] -> gScriptSet_046 */
    0x08662A78,  /* [ 47] -> gScriptSet_047 */
    0x08662E10,  /* [ 48] -> gScriptSet_048 */
    0x08663E6C,  /* [ 49] -> gScriptSet_049 */
    0x08664EF8,  /* [ 50] -> gScriptSet_050 */
    0x086666EC,  /* [ 51] -> gScriptSet_051 */
    0x08666BC4,  /* [ 52] -> gScriptSet_052 */
    0x08667B8C,  /* [ 53] -> gScriptSet_053 */
    0x0866822C,  /* [ 54] -> gScriptSet_054 */
    0x0866A5FC,  /* [ 55] -> gScriptSet_055 */
    0x0866CCA4,  /* [ 56] -> gScriptSet_056 */
    0x0866D73C,  /* [ 57] -> gScriptSet_057 */
    0x0866EC00,  /* [ 58] -> gScriptSet_058 */
    0x0866FC40,  /* [ 59] -> gScriptSet_059 */
    0x0866FCA8,  /* [ 60] -> gScriptSet_060 */
    0x0862D858,  /* [ 61] -> gScriptSet_Empty (空脚本集) */
    0x08671280,  /* [ 62] -> gScriptSet_062 */
    0x08672BAC,  /* [ 63] -> gScriptSet_063 */
    0x086755A0,  /* [ 64] -> gScriptSet_064 */
    0x08675F24,  /* [ 65] -> gScriptSet_065 */
    0x086768F4,  /* [ 66] -> gScriptSet_066 */
    0x08677078,  /* [ 67] -> gScriptSet_067 */
    0x08677338,  /* [ 68] -> gScriptSet_068 */
    0x08677FEC,  /* [ 69] -> gScriptSet_069 */
    0x086782E8,  /* [ 70] -> gScriptSet_070 */
    0x0867A390,  /* [ 71] -> gScriptSet_071 */
    0x0867B1B0,  /* [ 72] -> gScriptSet_072 */
    0x0862D858,  /* [ 73] -> gScriptSet_Empty (空脚本集) */
    0x0867BE94,  /* [ 74] -> gScriptSet_074 */
    0x0867C3F0,  /* [ 75] -> gScriptSet_075 */
    0x0867D4D8,  /* [ 76] -> gScriptSet_076 */
    0x0867FE24,  /* [ 77] -> gScriptSet_077 */
    0x086839DC,  /* [ 78] -> gScriptSet_078 */
    0x08687A54,  /* [ 79] -> gScriptSet_079 */
    0x0868A36C,  /* [ 80] -> gScriptSet_080 */
    0x0868EF0C,  /* [ 81] -> gScriptSet_081 */
    0x086910D8,  /* [ 82] -> gScriptSet_082 */
    0x08695B34,  /* [ 83] -> gScriptSet_083 */
    0x08695B80,  /* [ 84] -> gScriptSet_084 */
    0x08699208,  /* [ 85] -> gScriptSet_085 */
    0x0869BD34,  /* [ 86] -> gScriptSet_086 */
    0x0869E39C,  /* [ 87] -> gScriptSet_087 */
    0x0869FE24,  /* [ 88] -> gScriptSet_088 */
    0x086A1CD8,  /* [ 89] -> gScriptSet_089 */
    0x086A3908,  /* [ 90] -> gScriptSet_090 */
    0x086A68D8,  /* [ 91] -> gScriptSet_091 */
    0x086A7230,  /* [ 92] -> gScriptSet_092 */
    0x086A7F30,  /* [ 93] -> gScriptSet_093 */
    0x0862D858,  /* [ 94] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 95] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 96] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 97] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 98] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [ 99] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [100] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [101] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [102] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [103] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [104] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [105] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [106] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [107] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [108] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [109] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [110] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [111] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [112] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [113] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [114] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [115] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [116] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [117] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [118] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [119] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [120] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [121] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [122] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [123] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [124] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [125] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [126] -> gScriptSet_Empty (空脚本集) */
    0x086AA9C0,  /* [127] -> gScriptSet_127 */
    0x086AB488,  /* [128] -> gScriptSet_128 */
    0x0862D858,  /* [129] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [130] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [131] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [132] -> gScriptSet_Empty (空脚本集) */
    0x086AC614,  /* [133] -> gScriptSet_133 */
    0x0862D858,  /* [134] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [135] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [136] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [137] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [138] -> gScriptSet_Empty (空脚本集) */
    0x0862D858,  /* [139] -> gScriptSet_Empty (空脚本集) */
    0x0861C784,  /* [140] -> gScriptSet_140 */
    0x0861CC34,  /* [141] -> gScriptSet_141 */
    0x08624C34,  /* [142] -> gScriptSet_142 */
    0x087E610C,  /* [143] -> song_0063 (BGM/SFX) */
    0x087E6128,  /* [144] -> song_0064 (BGM/SFX) */
    0x087E6144,  /* [145] -> song_0065 (BGM/SFX) */
    0x087E6160,  /* [146] -> song_0066 (BGM/SFX) */
    0x087E617C,  /* [147] -> song_0067 (BGM/SFX) */
    0x087E6198,  /* [148] -> song_0068 (BGM/SFX) */
    0x087E61B4,  /* [149] -> song_0069 (BGM/SFX) */
    0x087E61D0,  /* [150] -> song_0070 (BGM/SFX) */
    0x087E61EC,  /* [151] -> song_0071 (BGM/SFX) */
    0x087E6208,  /* [152] -> song_0072 (BGM/SFX) */
    0x087E6224,  /* [153] -> song_0073 (BGM/SFX) */
    0x087E6240,  /* [154] -> song_0074 (BGM/SFX) */
    0x087E625C,  /* [155] -> song_0075 (BGM/SFX) */
    0x087E6278,  /* [156] -> song_0076 (BGM/SFX) */
    0x087E6294,  /* [157] -> song_0077 (BGM/SFX) */
    0x087E62B0,  /* [158] -> song_0078 (BGM/SFX) */
    0x087E62CC,  /* [159] -> song_0079 (BGM/SFX) */
    0x087E62E8,  /* [160] -> song_0080 (BGM/SFX) */
    0x087E6304,  /* [161] -> song_0081 (BGM/SFX) */
    0x087E6320,  /* [162] -> song_0082 (BGM/SFX) */
    0x087E6374,  /* [163] -> song_0085 (BGM/SFX) */
    0x087E6390,  /* [164] -> song_0086 (BGM/SFX) */
    0x087E6374,  /* [165] -> song_0085 (BGM/SFX) */
    0x087E6390,  /* [166] -> song_0086 (BGM/SFX) */
    0x087E63AC,  /* [167] -> song_0087 (BGM/SFX) */
    0x087E63C8,  /* [168] -> song_0088 (BGM/SFX) */
    0x087E63E4,  /* [169] -> song_0089 (BGM/SFX) */
    0x087E6404,  /* [170] -> song_0090 (BGM/SFX) */
    0x087E6420,  /* [171] -> song_0091 (BGM/SFX) */
    0x087E643C,  /* [172] -> song_0092 (BGM/SFX) */
    0x087E6458,  /* [173] -> song_0093 (BGM/SFX) */
    0x087E6474,  /* [174] -> song_0094 (BGM/SFX) */
    0x087E6490,  /* [175] -> song_0095 (BGM/SFX) */
    0x087E64AC,  /* [176] -> song_0096 (BGM/SFX) */
    0x087E64C8,  /* [177] -> song_0097 (BGM/SFX) */
    0x087E64E4,  /* [178] -> song_0098 (BGM/SFX) */
    0x087E6500,  /* [179] -> song_0099 (BGM/SFX) */
    0x087E6520,  /* [180] -> song_0100 (BGM/SFX) */
    0x087E653C,  /* [181] -> song_0101 (BGM/SFX) */
    0x087E6560,  /* [182] -> song_0102 (BGM/SFX) */
    0x087E6580,  /* [183] -> song_0103 (BGM/SFX) */
    0x087E65A0,  /* [184] -> song_0104 (BGM/SFX) */
    0x087E65C0,  /* [185] -> song_0105 (BGM/SFX) */
    0x087E65E0,  /* [186] -> song_0106 (BGM/SFX) */
    0x087E65FC,  /* [187] -> song_0107 (BGM/SFX) */
    0x087E661C,  /* [188] -> song_0108 (BGM/SFX) */
    0x087E6638,  /* [189] -> song_0109 (BGM/SFX) */
    0x087E6654,  /* [190] -> song_0110 (BGM/SFX) */
    0x087E6670,  /* [191] -> song_0111 (BGM/SFX) */
    0x087E668C,  /* [192] -> song_0112 (BGM/SFX) */
    0x087E66A8,  /* [193] -> song_0113 (BGM/SFX) */
    0x087E66C4,  /* [194] -> song_0114 (BGM/SFX) */
    0x087E66E0,  /* [195] -> song_0115 (BGM/SFX) */
    0x087E66FC,  /* [196] -> song_0116 (BGM/SFX) */
    0x087E6718,  /* [197] -> song_0117 (BGM/SFX) */
    0x087E6734,  /* [198] -> song_0118 (BGM/SFX) */
    0x087E6750,  /* [199] -> song_0119 (BGM/SFX) */
    0x087E676C,  /* [200] -> song_0120 (BGM/SFX) */
    0x087E6788,  /* [201] -> song_0121 (BGM/SFX) */
    0x087E67A4,  /* [202] -> song_0122 (BGM/SFX) */
    0x087E67C0,  /* [203] -> song_0123 (BGM/SFX) */
    0x087E67DC,  /* [204] -> song_0124 (BGM/SFX) */
    0x087E67F8,  /* [205] -> song_0125 (BGM/SFX) */
    0x087E6818,  /* [206] -> song_0126 (BGM/SFX) */
    0x087E6838,  /* [207] -> song_0127 (BGM/SFX) */
    0x087E6854,  /* [208] -> song_0128 (BGM/SFX) */
    0x087E6870,  /* [209] -> song_0129 (BGM/SFX) */
    0x087E688C,  /* [210] -> song_0130 (BGM/SFX) */
    0x087E68A8,  /* [211] -> song_0131 (BGM/SFX) */
    0x087E68C4,  /* [212] -> song_0132 (BGM/SFX) */
    0x087E68E0,  /* [213] -> song_0133 (BGM/SFX) */
    0x087E68FC,  /* [214] -> song_0134 (BGM/SFX) */
    0x087E6918,  /* [215] -> song_0135 (BGM/SFX) */
    0x087E6934,  /* [216] -> song_0136 (BGM/SFX) */
    0x087E6950,  /* [217] -> song_0137 (BGM/SFX) */
    0x087E696C,  /* [218] -> song_0138 (BGM/SFX) */
    0x087E6988,  /* [219] -> song_0139 (BGM/SFX) */
    0x087E69A4,  /* [220] -> song_0140 (BGM/SFX) */
    0x087E69C0,  /* [221] -> song_0141 (BGM/SFX) */
    0x087E69DC,  /* [222] -> song_0142 (BGM/SFX) */
    0x087E69F8,  /* [223] -> song_0143 (BGM/SFX) */
    0x087E6A14,  /* [224] -> song_0144 (BGM/SFX) */
    0x087E6A34,  /* [225] -> song_0145 (BGM/SFX) */
    0x087E6A50,  /* [226] -> song_0146 (BGM/SFX) */
    0x087E6A6C,  /* [227] -> song_0147 (BGM/SFX) */
    0x087E6A8C,  /* [228] -> song_0148 (BGM/SFX) */
    0x087E6AA8,  /* [229] -> song_0149 (BGM/SFX) */
    0x087E6AC4,  /* [230] -> song_0150 (BGM/SFX) */
    0x087E6AE0,  /* [231] -> song_0151 (BGM/SFX) */
    0x087E6B00,  /* [232] -> song_0152 (BGM/SFX) */
    0x087E6B1C,  /* [233] -> song_0153 (BGM/SFX) */
    0x087E6B38,  /* [234] -> song_0154 (BGM/SFX) */
    0x087E6B54,  /* [235] -> song_0155 (BGM/SFX) */
    0x087E6B70,  /* [236] -> song_0156 (BGM/SFX) */
    0x087E6B8C,  /* [237] -> song_0157 (BGM/SFX) */
    0x087E6BA8,  /* [238] -> song_0158 (BGM/SFX) */
    0x087E6BC4,  /* [239] -> song_0159 (BGM/SFX) */
    0x087E6BE0,  /* [240] -> song_0160 (BGM/SFX) */
    0x087E6BFC,  /* [241] -> song_0161 (BGM/SFX) */
    0x087E6C18,  /* [242] -> song_0162 (BGM/SFX) */
    0x087E6C34,  /* [243] -> song_0163 (BGM/SFX) */
    0x087E6C50,  /* [244] -> song_0164 (BGM/SFX) */
    0x087E6C6C,  /* [245] -> song_0165 (BGM/SFX) */
    0x087E6C88,  /* [246] -> song_0166 (BGM/SFX) */
    0x087E6CA4,  /* [247] -> song_0167 (BGM/SFX) */
    0x087E6CC0,  /* [248] -> song_0168 (BGM/SFX) */
    0x087E6CE0,  /* [249] -> song_0169 (BGM/SFX) */
    0x087E6CFC,  /* [250] -> song_0170 (BGM/SFX) */
    0x087E6D18,  /* [251] -> song_0171 (BGM/SFX) */
    0x087E6D34,  /* [252] -> song_0172 (BGM/SFX) */
    0x087E6D50,  /* [253] -> song_0173 (BGM/SFX) */
    0x087E6D6C,  /* [254] -> song_0174 (BGM/SFX) */
    0x087E6D88,  /* [255] -> song_0175 (BGM/SFX) */
    0x087E6DA4,  /* [256] -> song_0176 (BGM/SFX) */
    0x087E6DC0,  /* [257] -> song_0177 (BGM/SFX) */
    0x087E6DDC,  /* [258] -> song_0178 (BGM/SFX) */
    0x087E6DFC,  /* [259] -> song_0179 (BGM/SFX) */
    0x087E6E18,  /* [260] -> song_0180 (BGM/SFX) */
    0x087E6E38,  /* [261] -> song_0181 (BGM/SFX) */
    0x087E6E54,  /* [262] -> song_0182 (BGM/SFX) */
    0x087E6E70,  /* [263] -> song_0183 (BGM/SFX) */
    0x087E6E8C,  /* [264] -> song_0184 (BGM/SFX) */
    0x087E6EA8,  /* [265] -> song_0185 (BGM/SFX) */
    0x087E6EC4,  /* [266] -> song_0186 (BGM/SFX) */
    0x087E6EE0,  /* [267] -> song_0187 (BGM/SFX) */
    0x087E6F04,  /* [268] -> song_0188 (BGM/SFX) */
    0x087E6F20,  /* [269] -> song_0189 (BGM/SFX) */
    0x087E6F3C,  /* [270] -> song_0190 (BGM/SFX) */
    0x087E6F58,  /* [271] -> song_0191 (BGM/SFX) */
    0x087E6F74,  /* [272] -> song_0192 (BGM/SFX) */
    0x087E6F90,  /* [273] -> song_0193 (BGM/SFX) */
    0x087E6FAC,  /* [274] -> song_0194 (BGM/SFX) */
    0x087E6FC8,  /* [275] -> song_0195 (BGM/SFX) */
    0x087E6FE4,  /* [276] -> song_0196 (BGM/SFX) */
    0x087E7000,  /* [277] -> song_0197 (BGM/SFX) */
    0x087E701C,  /* [278] -> song_0198 (BGM/SFX) */
    0x087E7038,  /* [279] -> song_0199 (BGM/SFX) */
    0x087E7054,  /* [280] -> song_0200 (BGM/SFX) */
    0x087E7070,  /* [281] -> song_0201 (BGM/SFX) */
    0x087E708C,  /* [282] -> song_0202 (BGM/SFX) */
    0x087E70A8,  /* [283] -> song_0203 (BGM/SFX) */
    0x087E70C4,  /* [284] -> song_0204 (BGM/SFX) */
    0x087E70E0,  /* [285] -> song_0205 (BGM/SFX) */
    0x087E70FC,  /* [286] -> song_0206 (BGM/SFX) */
    0x087E7118,  /* [287] -> song_0207 (BGM/SFX) */
    0x087E7140,  /* [288] -> song_0208 (BGM/SFX) */
    0x087E715C,  /* [289] -> song_0209 (BGM/SFX) */
    0x087E7178,  /* [290] -> song_0210 (BGM/SFX) */
    0x087E7194,  /* [291] -> song_0211 (BGM/SFX) */
    0x087E71B4,  /* [292] -> song_0212 (BGM/SFX) */
    0x087E71D0,  /* [293] -> song_0213 (BGM/SFX) */
    0x087E71F4,  /* [294] -> song_0214 (BGM/SFX) */
    0x087E7210,  /* [295] -> song_0215 (BGM/SFX) */
    0x087E7234,  /* [296] -> song_0216 (BGM/SFX) */
    0x087E7250,  /* [297] -> song_0217 (BGM/SFX) */
    0x087E726C,  /* [298] -> song_0218 (BGM/SFX) */
    0x087E7288,  /* [299] -> song_0219 (BGM/SFX) */
    0x087E72A4,  /* [300] -> song_0220 (BGM/SFX) */
    0x087E72C0,  /* [301] -> song_0221 (BGM/SFX) */
    0x087E72DC,  /* [302] -> song_0222 (BGM/SFX) */
    0x087E72FC,  /* [303] -> song_0223 (BGM/SFX) */
    0x087E7318,  /* [304] -> song_0224 (BGM/SFX) */
    0x087E7334,  /* [305] -> song_0225 (BGM/SFX) */
    0x087E7350,  /* [306] -> song_0226 (BGM/SFX) */
    0x087E736C,  /* [307] -> song_0227 (BGM/SFX) */
    0x087E7388,  /* [308] -> song_0228 (BGM/SFX) */
    0x087E73A4,  /* [309] -> song_0229 (BGM/SFX) */
    0x087E73C0,  /* [310] -> song_0230 (BGM/SFX) */
    0x087E73DC,  /* [311] -> song_0231 (BGM/SFX) */
    0x087E73F8,  /* [312] -> song_0232 (BGM/SFX) */
    0x087E7414,  /* [313] -> song_0233 (BGM/SFX) */
    0x087E7430,  /* [314] -> song_0234 (BGM/SFX) */
    0x087E744C,  /* [315] -> song_0235 (BGM/SFX) */
    0x087E7468,  /* [316] -> song_0236 (BGM/SFX) */
    0x087E7484,  /* [317] -> song_0237 (BGM/SFX) */
    0x087E74A0,  /* [318] -> song_0238 (BGM/SFX) */
    0x087E74BC,  /* [319] -> song_0239 (BGM/SFX) */
    0x087E74D8,  /* [320] -> song_0240 (BGM/SFX) */
    0x087E74F4,  /* [321] -> song_0241 (BGM/SFX) */
    0x087E7510,  /* [322] -> song_0242 (BGM/SFX) */
    0x087E752C,  /* [323] -> song_0243 (BGM/SFX) */
    0x087E754C,  /* [324] -> song_0244 (BGM/SFX) */
    0x087E7568,  /* [325] -> song_0245 (BGM/SFX) */
    0x087E7584,  /* [326] -> song_0246 (BGM/SFX) */
    0x087E75A0,  /* [327] -> song_0247 (BGM/SFX) */
    0x087E75BC,  /* [328] -> song_0248 (BGM/SFX) */
    0x087E75D8,  /* [329] -> song_0249 (BGM/SFX) */
    0x087E75F4,  /* [330] -> song_0250 (BGM/SFX) */
    0x087E7610,  /* [331] -> song_0251 (BGM/SFX) */
    0x087E762C,  /* [332] -> song_0252 (BGM/SFX) */
    0x087E7648,  /* [333] -> song_0253 (BGM/SFX) */
    0x087E7664,  /* [334] -> song_0254 (BGM/SFX) */
    0x087E7680,  /* [335] -> song_0255 (BGM/SFX) */
    0x087E769C,  /* [336] -> song_0256 (BGM/SFX) */
    0x087E76B8,  /* [337] -> song_0257 (BGM/SFX) */
    0x087E76D4,  /* [338] -> song_0258 (BGM/SFX) */
    0x087E76F0,  /* [339] -> song_0259 (BGM/SFX) */
    0x087E770C,  /* [340] -> song_0260 (BGM/SFX) */
    0x087E7728,  /* [341] -> song_0261 (BGM/SFX) */
    0x087E7744,  /* [342] -> song_0262 (BGM/SFX) */
    0x087E7760,  /* [343] -> song_0263 (BGM/SFX) */
    0x087E7780,  /* [344] -> song_0264 (BGM/SFX) */
    0x087E77A0,  /* [345] -> song_0265 (BGM/SFX) */
    0x087E77BC,  /* [346] -> song_0266 (BGM/SFX) */
    0x087E77D8,  /* [347] -> song_0267 (BGM/SFX) */
    0x087E77F4,  /* [348] -> song_0268 (BGM/SFX) */
    0x087E7810,  /* [349] -> song_0269 (BGM/SFX) */
    0x087E782C,  /* [350] -> song_0270 (BGM/SFX) */
    0x087E7848,  /* [351] -> song_0271 (BGM/SFX) */
    0x087E7864,  /* [352] -> song_0272 (BGM/SFX) */
    0x087E7880,  /* [353] -> song_0273 (BGM/SFX) */
    0x087E789C,  /* [354] -> song_0274 (BGM/SFX) */
    0x087E78B8,  /* [355] -> song_0275 (BGM/SFX) */
    0x087E78D4,  /* [356] -> song_0276 (BGM/SFX) */
    0x087E78F0,  /* [357] -> song_0277 (BGM/SFX) */
    0x087E790C,  /* [358] -> song_0278 (BGM/SFX) */
    0x087E7928,  /* [359] -> song_0279 (BGM/SFX) */
    0x087E7928,  /* [360] -> song_0279 (BGM/SFX) */
    0x087E82B8,  /* [361] -> song_0284 (BGM/SFX) */
    0x087E7948  /* [362] -> song_0280 (BGM/SFX) */
};
