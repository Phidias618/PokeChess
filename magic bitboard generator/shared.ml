

type shared_tree =
  | Bishop of int * int * (shared_tree list)
  | Rook of int * int * (shared_tree list) 
;;

let rec rotate_clockwise shared =
  match shared with
  | Bishop (x, y, next) -> Bishop (y, 7 - x, List.map rotate_clockwise next)
  | Rook   (x, y, next) -> Rook   (y, 7 - x, List.map rotate_clockwise next) 
;;

let rec rotate_trig shared =
  match shared with
  | Bishop (x, y, next) -> Bishop (7 - y, x, List.map rotate_trig next)
  | Rook   (x, y, next) -> Rook   (7 - y, x, List.map rotate_trig next) 
;;

let rec hflip shared =
  match shared with
  | Bishop (x, y, next) -> Bishop (x, 7 - y, List.map hflip next)
  | Rook (x, y, next) -> Rook (x, 7 - y, List.map hflip next)
;;

let rec vflip shared =
  match shared with
  | Bishop (x, y, next) -> Bishop (7 - x, y, List.map vflip next)
  | Rook (x, y, next) -> Rook (7 - x, y, List.map vflip next)
;;

let rook_1_1 =
Rook (0, 7, [
  Rook (1, 6, [
    Bishop (5, 2, [
      Bishop (0, 6, [
        Bishop (1, 7, []);
      ]);
    ]);
    Bishop (6, 1, []);
    Bishop (7, 0, []);
  ]);
]);;

let rook_1_2 =
Rook (1, 7, [
  Rook (0, 6, []);
]);;


let rook_2_1 = 
Rook (2, 7, [
  Rook (3, 6, [
    Bishop (3, 7, [
      Bishop (7, 2, []);
    ]);
    Bishop (2, 6, []);
  ]);
]);;

let rook_2_2 =
Rook (3, 7, [
  Rook (2, 6, [
    Bishop (3, 6, []);
  ]);
]);;

let rook_3_1 =
Rook (2, 5, [
  Rook (3, 4, [
    Bishop(2, 4, []);
    Bishop(3, 5, []);
  ]);
]);;

let rook_3_2 =
Rook (2, 4, [
  Rook (3, 5, [
    Bishop (3, 4, []);
  ]);
]);;

let best_bishop_ind = [|
  [|5; 4; 5; 5; 5; 5; 4; 5|];
  [|4; 4; 5; 5; 5; 5; 4; 4|];
  [|4; 4; 7; 7; 7; 7; 4; 4|];
  [|5; 5; 7; 9; 9; 7; 5; 5|];
  [|5; 5; 7; 9; 9; 7; 5; 5|];
  [|4; 4; 7; 7; 7; 7; 4; 4|];
  [|4; 4; 5; 5; 5; 5; 4; 4|];
  [|5; 4; 5; 5; 5; 5; 4; 5|];
|];;

let best_rook_ind = [|
  [|12; 11; 11; 11; 11; 11; 11; 12|];
  [|11; 10; 10; 10; 10; 10; 10; 11|];
  [|11; 10; 10; 10; 10; 10; 10; 11|];
  [|11; 10; 10; 10; 10; 10; 10; 11|];
  [|11; 10; 10; 10; 10; 10; 10; 11|];
  [|11; 10; 10; 10; 10; 10; 10; 11|];
  [|10;  9;  9;  9;  9;  9;  9; 10|];
  [|11; 10; 10; 10; 10; 11; 10; 11|];
|];;


let rook_magic = [|
        [|0x0280004002816010L; 0x0840100420044000L; 0x0100090044102001L; 0x0880080180100004L; 0x0300040800500300L; 0x2080018012005400L; 0x4280048006000100L; 0x0100120640822100L; |];
        [|0x071d802040028000L; 0x2020400020005000L; 0x5800801000802002L; 0x0001002010000900L; 0x8010800800040281L; 0x0009800201140080L; 0x0004800100020080L; 0x8018800100005080L; |];
        [|0x0020008080004000L; 0x0000464002201001L; 0x0203010012442000L; 0x1001010008100420L; 0x8000808008000400L; 0x5240818004000e00L; 0x0050040001029008L; 0x0040020001108044L; |];
        [|0x1140004180008028L; 0x00104000c0201000L; 0x0020001010040200L; 0x0412080080100280L; 0x0021018500080010L; 0x1004010040020040L; 0x0a14108400020801L; 0x0110802080084500L; |];
        [|0x0402008102002044L; 0x0000806001804004L; 0x4301002001001040L; 0x4520828804801000L; 0x0500040080800802L; 0x8021800401800200L; 0x2001000405001200L; 0x1101040042000081L; |];
        [|0x4005804004658000L; 0x0002002100820041L; 0x0801001020010040L; 0x0009000810010020L; 0x0114008008008004L; 0x2482001004020008L; 0x6801221008040041L; 0x00000c8441220004L; |];
        [|0x48fffe99fecfaa00L; 0x48fffe99fecfaa00L; 0x497fffadff9c2e00L; 0x613fffddffce9200L; 0xffffffe9ffe7ce00L; 0xfffffff5fff3e600L; 0x0003ff95e5e6a4c0L; 0x510ffff5f63c96a0L; |];
        [|0xebffffb9ff9fc526L; 0x61fffeddfeedaeaeL; 0x53bfffedffdeb1a2L; 0x127fffb9ffdfb5f6L; 0x411fffddffdbf4d6L; 0x8002000110080402L; 0x0003ffef27eebe74L; 0x7645FFFECBFEA79EL; |];
|];;

let bishop_magic = [|
        [|0xffedf9fd7cfcffffL; 0xfc0962854a77f576L; 0x2812089401090000L; 0x0008060042020044L; 0x0002021101288880L; 0x40120202200018c0L; 0xfc0a66c64a7ef576L; 0x7ffdfdfcbd79ffffL; |];
        [|0xfc0846a64a34fff6L; 0xfc087a874a3cf7f6L; 0x402010708c810000L; 0x0400c40404820043L; 0x9281084840001184L; 0x4400022820080802L; 0xfc0864ae59b4ff76L; 0x3c0860af4b35ff76L; |];
        [|0x73c01af56cf4cffbL; 0x41a01cfad64aaffcL; 0x8408004112040011L; 0x9800803802810003L; 0x1002800400e00a02L; 0x4422000922100200L; 0x7c0c028f5b34ff76L; 0xfc0a028e5ab4df76L; |];
        [|0x4920080004900440L; 0x2009210010044100L; 0x6802240212080200L; 0x4804080300220040L; 0x180404001041004cL; 0x8810012050c41004L; 0x00a08420420a1224L; 0x0006020208c04641L; |];
        [|0x881004040010b108L; 0x0201243004200922L; 0x0002012200142800L; 0xb040202020080080L; 0x6040008020920020L; 0x00a0408100028044L; 0x2151080206030500L; 0x1484010204602091L; |];
        [|0xdcefd9b54bfcc09fL; 0xf95ffa765afd602bL; 0x8040084402201000L; 0x042468a011140800L; 0x0004081010400402L; 0x4004212546000100L; 0x43ff9a5cf4ca0c01L; 0x4bffcd8e7c587601L; |];
        [|0xfc0ff2865334f576L; 0xfc0bf6ce5924f576L; 0x80140c8088210400L; 0x160400a084040100L; 0x0040004059220009L; 0x0a88041002120120L; 0xc3ffb7dc36ca8c89L; 0xc3ff8a54f4ca2c89L; |];
        [|0xfffffcfcfd79edffL; 0xfc0863fccb147576L; 0x000204030080d002L; 0xe028020000420880L; 0x2086010010420220L; 0x8094010404082208L; 0xfc087e8e4bb2f736L; 0x43ff9e4ef4ca2c89L; |];
|];;


let bishop_premask = [|
  [|(Int64.of_string "0x0040201008040200"); (Int64.of_string "0x0000402010080400"); (Int64.of_string "0x0000004020100a00"); (Int64.of_string "0x0000000040221400"); (Int64.of_string "0x0000000002442800"); (Int64.of_string "0x0000000204085000"); (Int64.of_string "0x0000020408102000"); (Int64.of_string "0x0002040810204000"); |];
  [|(Int64.of_string "0x0020100804020000"); (Int64.of_string "0x0040201008040000"); (Int64.of_string "0x00004020100a0000"); (Int64.of_string "0x0000004022140000"); (Int64.of_string "0x0000000244280000"); (Int64.of_string "0x0000020408500000"); (Int64.of_string "0x0002040810200000"); (Int64.of_string "0x0004081020400000"); |];
  [|(Int64.of_string "0x0010080402000200"); (Int64.of_string "0x0020100804000400"); (Int64.of_string "0x004020100a000a00"); (Int64.of_string "0x0000402214001400"); (Int64.of_string "0x0000024428002800"); (Int64.of_string "0x0002040850005000"); (Int64.of_string "0x0004081020002000"); (Int64.of_string "0x0008102040004000"); |];
  [|(Int64.of_string "0x0008040200020400"); (Int64.of_string "0x0010080400040800"); (Int64.of_string "0x0020100a000a1000"); (Int64.of_string "0x0040221400142200"); (Int64.of_string "0x0002442800284400"); (Int64.of_string "0x0004085000500800"); (Int64.of_string "0x0008102000201000"); (Int64.of_string "0x0010204000402000"); |];
  [|(Int64.of_string "0x0004020002040800"); (Int64.of_string "0x0008040004081000"); (Int64.of_string "0x00100a000a102000"); (Int64.of_string "0x0022140014224000"); (Int64.of_string "0x0044280028440200"); (Int64.of_string "0x0008500050080400"); (Int64.of_string "0x0010200020100800"); (Int64.of_string "0x0020400040201000"); |];
  [|(Int64.of_string "0x0002000204081000"); (Int64.of_string "0x0004000408102000"); (Int64.of_string "0x000a000a10204000"); (Int64.of_string "0x0014001422400000"); (Int64.of_string "0x0028002844020000"); (Int64.of_string "0x0050005008040200"); (Int64.of_string "0x0020002010080400"); (Int64.of_string "0x0040004020100800"); |];
  [|(Int64.of_string "0x0000020408102000"); (Int64.of_string "0x0000040810204000"); (Int64.of_string "0x00000a1020400000"); (Int64.of_string "0x0000142240000000"); (Int64.of_string "0x0000284402000000"); (Int64.of_string "0x0000500804020000"); (Int64.of_string "0x0000201008040200"); (Int64.of_string "0x0000402010080400"); |];
  [|(Int64.of_string "0x0002040810204000"); (Int64.of_string "0x0004081020400000"); (Int64.of_string "0x000a102040000000"); (Int64.of_string "0x0014224000000000"); (Int64.of_string "0x0028440200000000"); (Int64.of_string "0x0050080402000000"); (Int64.of_string "0x0020100804020000"); (Int64.of_string "0x0040201008040200"); |];
|];;

let rook_premask = [|
  [|(Int64.of_string "0x000101010101017e"); (Int64.of_string "0x000202020202027c"); (Int64.of_string "0x000404040404047a"); (Int64.of_string "0x0008080808080876"); (Int64.of_string "0x001010101010106e"); (Int64.of_string "0x002020202020205e"); (Int64.of_string "0x004040404040403e"); (Int64.of_string "0x008080808080807e"); |];
  [|(Int64.of_string "0x0001010101017e00"); (Int64.of_string "0x0002020202027c00"); (Int64.of_string "0x0004040404047a00"); (Int64.of_string "0x0008080808087600"); (Int64.of_string "0x0010101010106e00"); (Int64.of_string "0x0020202020205e00"); (Int64.of_string "0x0040404040403e00"); (Int64.of_string "0x0080808080807e00"); |];
  [|(Int64.of_string "0x00010101017e0100"); (Int64.of_string "0x00020202027c0200"); (Int64.of_string "0x00040404047a0400"); (Int64.of_string "0x0008080808760800"); (Int64.of_string "0x00101010106e1000"); (Int64.of_string "0x00202020205e2000"); (Int64.of_string "0x00404040403e4000"); (Int64.of_string "0x00808080807e8000"); |];
  [|(Int64.of_string "0x000101017e010100"); (Int64.of_string "0x000202027c020200"); (Int64.of_string "0x000404047a040400"); (Int64.of_string "0x0008080876080800"); (Int64.of_string "0x001010106e101000"); (Int64.of_string "0x002020205e202000"); (Int64.of_string "0x004040403e404000"); (Int64.of_string "0x008080807e808000"); |];
  [|(Int64.of_string "0x0001017e01010100"); (Int64.of_string "0x0002027c02020200"); (Int64.of_string "0x0004047a04040400"); (Int64.of_string "0x0008087608080800"); (Int64.of_string "0x0010106e10101000"); (Int64.of_string "0x0020205e20202000"); (Int64.of_string "0x0040403e40404000"); (Int64.of_string "0x0080807e80808000"); |];
  [|(Int64.of_string "0x00017e0101010100"); (Int64.of_string "0x00027c0202020200"); (Int64.of_string "0x00047a0404040400"); (Int64.of_string "0x0008760808080800"); (Int64.of_string "0x00106e1010101000"); (Int64.of_string "0x00205e2020202000"); (Int64.of_string "0x00403e4040404000"); (Int64.of_string "0x00807e8080808000"); |];
  [|(Int64.of_string "0x007e010101010100"); (Int64.of_string "0x007c020202020200"); (Int64.of_string "0x007a040404040400"); (Int64.of_string "0x0076080808080800"); (Int64.of_string "0x006e101010101000"); (Int64.of_string "0x005e202020202000"); (Int64.of_string "0x003e404040404000"); (Int64.of_string "0x007e808080808000"); |];
  [|(Int64.of_string "0x7e01010101010100"); (Int64.of_string "0x7c02020202020200"); (Int64.of_string "0x7a04040404040400"); (Int64.of_string "0x7608080808080800"); (Int64.of_string "0x6e10101010101000"); (Int64.of_string "0x5e20202020202000"); (Int64.of_string "0x3e40404040404000"); (Int64.of_string "0x7e80808080808000"); |];
|];;

let rook_postmask = [|
  [|(Int64.of_string "0x01010101010101fe"); (Int64.of_string "0x02020202020202fd"); (Int64.of_string "0x04040404040404fb"); (Int64.of_string "0x08080808080808f7"); (Int64.of_string "0x10101010101010ef"); (Int64.of_string "0x20202020202020df"); (Int64.of_string "0x40404040404040bf"); (Int64.of_string "0x808080808080807f"); |];
  [|(Int64.of_string "0x010101010101fe01"); (Int64.of_string "0x020202020202fd02"); (Int64.of_string "0x040404040404fb04"); (Int64.of_string "0x080808080808f708"); (Int64.of_string "0x101010101010ef10"); (Int64.of_string "0x202020202020df20"); (Int64.of_string "0x404040404040bf40"); (Int64.of_string "0x8080808080807f80"); |];
  [|(Int64.of_string "0x0101010101fe0101"); (Int64.of_string "0x0202020202fd0202"); (Int64.of_string "0x0404040404fb0404"); (Int64.of_string "0x0808080808f70808"); (Int64.of_string "0x1010101010ef1010"); (Int64.of_string "0x2020202020df2020"); (Int64.of_string "0x4040404040bf4040"); (Int64.of_string "0x80808080807f8080"); |];
  [|(Int64.of_string "0x01010101fe010101"); (Int64.of_string "0x02020202fd020202"); (Int64.of_string "0x04040404fb040404"); (Int64.of_string "0x08080808f7080808"); (Int64.of_string "0x10101010ef101010"); (Int64.of_string "0x20202020df202020"); (Int64.of_string "0x40404040bf404040"); (Int64.of_string "0x808080807f808080"); |];
  [|(Int64.of_string "0x010101fe01010101"); (Int64.of_string "0x020202fd02020202"); (Int64.of_string "0x040404fb04040404"); (Int64.of_string "0x080808f708080808"); (Int64.of_string "0x101010ef10101010"); (Int64.of_string "0x202020df20202020"); (Int64.of_string "0x404040bf40404040"); (Int64.of_string "0x8080807f80808080"); |];
  [|(Int64.of_string "0x0101fe0101010101"); (Int64.of_string "0x0202fd0202020202"); (Int64.of_string "0x0404fb0404040404"); (Int64.of_string "0x0808f70808080808"); (Int64.of_string "0x1010ef1010101010"); (Int64.of_string "0x2020df2020202020"); (Int64.of_string "0x4040bf4040404040"); (Int64.of_string "0x80807f8080808080"); |];
  [|(Int64.of_string "0x01fe010101010101"); (Int64.of_string "0x02fd020202020202"); (Int64.of_string "0x04fb040404040404"); (Int64.of_string "0x08f7080808080808"); (Int64.of_string "0x10ef101010101010"); (Int64.of_string "0x20df202020202020"); (Int64.of_string "0x40bf404040404040"); (Int64.of_string "0x807f808080808080"); |];
  [|(Int64.of_string "0xfe01010101010101"); (Int64.of_string "0xfd02020202020202"); (Int64.of_string "0xfb04040404040404"); (Int64.of_string "0xf708080808080808"); (Int64.of_string "0xef10101010101010"); (Int64.of_string "0xdf20202020202020"); (Int64.of_string "0xbf40404040404040"); (Int64.of_string "0x7f80808080808080"); |];
|];;


let bishop_postmask = [|
  [|(Int64.of_string "0x8040201008040200"); (Int64.of_string "0x0080402010080500"); (Int64.of_string "0x0000804020110a00"); (Int64.of_string "0x0000008041221400"); (Int64.of_string "0x0000000182442800"); (Int64.of_string "0x0000010204885000"); (Int64.of_string "0x000102040810a000"); (Int64.of_string "0x0102040810204000"); |];
  [|(Int64.of_string "0x4020100804020002"); (Int64.of_string "0x8040201008050005"); (Int64.of_string "0x00804020110a000a"); (Int64.of_string "0x0000804122140014"); (Int64.of_string "0x0000018244280028"); (Int64.of_string "0x0001020488500050"); (Int64.of_string "0x0102040810a000a0"); (Int64.of_string "0x0204081020400040"); |];
  [|(Int64.of_string "0x2010080402000204"); (Int64.of_string "0x4020100805000508"); (Int64.of_string "0x804020110a000a11"); (Int64.of_string "0x0080412214001422"); (Int64.of_string "0x0001824428002844"); (Int64.of_string "0x0102048850005088"); (Int64.of_string "0x02040810a000a010"); (Int64.of_string "0x0408102040004020"); |];
  [|(Int64.of_string "0x1008040200020408"); (Int64.of_string "0x2010080500050810"); (Int64.of_string "0x4020110a000a1120"); (Int64.of_string "0x8041221400142241"); (Int64.of_string "0x0182442800284482"); (Int64.of_string "0x0204885000508804"); (Int64.of_string "0x040810a000a01008"); (Int64.of_string "0x0810204000402010"); |];
  [|(Int64.of_string "0x0804020002040810"); (Int64.of_string "0x1008050005081020"); (Int64.of_string "0x20110a000a112040"); (Int64.of_string "0x4122140014224180"); (Int64.of_string "0x8244280028448201"); (Int64.of_string "0x0488500050880402"); (Int64.of_string "0x0810a000a0100804"); (Int64.of_string "0x1020400040201008"); |];
  [|(Int64.of_string "0x0402000204081020"); (Int64.of_string "0x0805000508102040"); (Int64.of_string "0x110a000a11204080"); (Int64.of_string "0x2214001422418000"); (Int64.of_string "0x4428002844820100"); (Int64.of_string "0x8850005088040201"); (Int64.of_string "0x10a000a010080402"); (Int64.of_string "0x2040004020100804"); |];
  [|(Int64.of_string "0x0200020408102040"); (Int64.of_string "0x0500050810204080"); (Int64.of_string "0x0a000a1120408000"); (Int64.of_string "0x1400142241800000"); (Int64.of_string "0x2800284482010000"); (Int64.of_string "0x5000508804020100"); (Int64.of_string "0xa000a01008040201"); (Int64.of_string "0x4000402010080402"); |];
  [|(Int64.of_string "0x0002040810204080"); (Int64.of_string "0x0005081020408000"); (Int64.of_string "0x000a112040800000"); (Int64.of_string "0x0014224180000000"); (Int64.of_string "0x0028448201000000"); (Int64.of_string "0x0050880402010000"); (Int64.of_string "0x00a0100804020100"); (Int64.of_string "0x0040201008040201"); |];
|];;

let king_attack_patern = [|
  [| 0x0000000000000302L; 0x0000000000000705L; 0x0000000000000e0aL; 0x0000000000001c14L; 0x0000000000003828L; 0x0000000000007050L; 0x000000000000e0a0L; 0x000000000000c040L; |];
  [| 0x0000000000030203L; 0x0000000000070507L; 0x00000000000e0a0eL; 0x00000000001c141cL; 0x0000000000382838L; 0x0000000000705070L; 0x0000000000e0a0e0L; 0x0000000000c040c0L; |];
  [| 0x0000000003020300L; 0x0000000007050700L; 0x000000000e0a0e00L; 0x000000001c141c00L; 0x0000000038283800L; 0x0000000070507000L; 0x00000000e0a0e000L; 0x00000000c040c000L; |];
  [| 0x0000000302030000L; 0x0000000705070000L; 0x0000000e0a0e0000L; 0x0000001c141c0000L; 0x0000003828380000L; 0x0000007050700000L; 0x000000e0a0e00000L; 0x000000c040c00000L; |];
  [| 0x0000030203000000L; 0x0000070507000000L; 0x00000e0a0e000000L; 0x00001c141c000000L; 0x0000382838000000L; 0x0000705070000000L; 0x0000e0a0e0000000L; 0x0000c040c0000000L; |];
  [| 0x0003020300000000L; 0x0007050700000000L; 0x000e0a0e00000000L; 0x001c141c00000000L; 0x0038283800000000L; 0x0070507000000000L; 0x00e0a0e000000000L; 0x00c040c000000000L; |];
  [| 0x0302030000000000L; 0x0705070000000000L; 0x0e0a0e0000000000L; 0x1c141c0000000000L; 0x3828380000000000L; 0x7050700000000000L; 0xe0a0e00000000000L; 0xc040c00000000000L; |];
  [| 0x0203000000000000L; 0x0507000000000000L; 0x0a0e000000000000L; 0x141c000000000000L; 0x2838000000000000L; 0x5070000000000000L; 0xa0e0000000000000L; 0x40c0000000000000L; |];
|];;


let ( << ) = ( lsl );;
let ( >> ) = ( lsr );;

let get_weight shared =
  match shared with
  | Bishop (x, y, _) -> 1 << best_bishop_ind.(y).(x)
  | Rook (x, y, _) -> 1 << best_rook_ind.(y).(x)
;;

let rec union_mask shared =
  match shared with
  | Bishop (x, y, next) -> List.fold_left (fun acc current -> Int64.logor acc (union_mask current)) bishop_postmask.(y).(x) next
  | Rook   (x, y, next) -> List.fold_left (fun acc current -> Int64.logor acc (union_mask current)) rook_postmask.(y).(x) next
;;

let get_postmask shared =
  match shared with
  | Rook (x, y, _) -> rook_postmask.(y).(x)
  | Bishop (x, y, _) -> bishop_postmask.(y).(x)
;;

let get_name shared =
  match shared with
  | Rook _ -> "Rook"
  | Bishop _ -> "Bishop"
;;

let rec is_valid shared =
  let rec aux shared mask =
    match shared with
    | Rook (x, y, next) | Bishop (x, y, next)
    -> 
      let weight = (get_weight shared) in
      let my_mask = get_postmask shared in
      (List.fold_left (fun a b -> a && (aux b (Int64.logor mask my_mask))) true next) &&
      weight >= (List.fold_left (fun a b -> a + get_weight b) 0 next) &&
      let inter_mask = Int64.logand my_mask (Int64.logor mask (List.fold_left (fun acc current -> Int64.logor acc (union_mask current)) 0L next)) in
      let neighbor_mask = king_attack_patern.(y).(x) in
      if (Int64.logand neighbor_mask inter_mask) = inter_mask
      then
        true
      else
        (Printf.printf "error %s(%d, %d), %s\n" (get_name shared) x y (int64_to_hex inter_mask);
        false)
  in aux shared 0L
;;

let int64_to_hex n =
  let str = ref "" in
  let n' = ref n in
  for i = 15 downto 0 do
    let digit = Int64.to_int (Int64.logand (!n') 0b1111L) in
    str := (if digit < 10 then (string_of_int digit) else (String.make 1 (Char.chr ((Char.code 'a') - 10 + digit)))) ^ !str;
    
    n' := Int64.shift_right_logical (!n') 4
  done;
  "0x" ^ !str
;;

let () =
  let all_rook = [|
    rook_1_1; hflip rook_1_1; vflip rook_1_1; hflip (vflip rook_1_1);
    rook_1_2; hflip rook_1_2; vflip rook_1_2; hflip (vflip rook_1_2);
    rook_2_1; hflip rook_2_1; vflip rook_2_1; hflip (vflip rook_2_1); rotate_clockwise rook_2_1; rotate_clockwise (hflip rook_2_1); rotate_clockwise (vflip rook_2_1); rotate_clockwise (hflip (vflip rook_2_1));
    rook_2_2; hflip rook_2_2; vflip rook_2_2; hflip (vflip rook_2_2); rotate_clockwise rook_2_2; rotate_clockwise (hflip rook_2_2); rotate_clockwise (vflip rook_2_2); rotate_clockwise (hflip (vflip rook_2_2));
    rook_3_1; hflip rook_3_1; vflip rook_3_1; hflip (vflip rook_3_1);
    rook_3_2; hflip rook_3_2; vflip rook_3_2; hflip (vflip rook_3_2);
  |] in
  Array.iter (fun shared -> assert(is_valid shared)) all_rook;
  
  let bishop_table = Array.make_matrix 8 8 (-1) in
  let rook_table = Array.make_matrix 8 8 (-1) in
  
  let table_index = ref 0 in
  for i = 0 to Array.length all_rook - 1 do
    let shared = all_rook.(i) in
    
    let rec explore current = 
      match current with
      | Rook (x, y, next) ->
        rook_table.(y).(x) <- !table_index;
        let new_index = !table_index + get_weight current in
        List.iter explore next;
        if !table_index > new_index then failwith "Ill shared memory";
        table_index := new_index
      | Bishop (x, y, next) ->
        bishop_table.(y).(x) <- !table_index;
        let new_index = !table_index + get_weight current in
        List.iter explore next;
        if !table_index > new_index then failwith "Ill shared memory";
        table_index := new_index
    in explore shared;
  done;
  Printf.printf "Table size : %d\n" !table_index;
  
  Printf.printf "\nMagicBitboardData const rook_magic[64] = {\n";
  for y = 0 to 7 do
    for x = 0 to 7 do
      Printf.printf "\t{ %sull, %sull, %sull, %d, %d }, \n" (int64_to_hex rook_premask.(y).(x)) (int64_to_hex rook_postmask.(y).(x)) (int64_to_hex rook_magic.(y).(x)) rook_table.(y).(x) (64 - best_rook_ind.(y).(x))
    done;
    Printf.printf "\n";
  done;
  Printf.printf "};\n";
  
  Printf.printf "\nMagicBitboardData const bishop_magic[64] = {\n";
  for y = 0 to 7 do
    for x = 0 to 7 do
      Printf.printf "\t{ %sull, %sull, %sull, %d, %d }, \n" (int64_to_hex bishop_premask.(y).(x)) (int64_to_hex bishop_postmask.(y).(x)) (int64_to_hex bishop_magic.(y).(x)) bishop_table.(y).(x) (64 - best_bishop_ind.(y).(x))
    done;
    Printf.printf "\n";
  done;
  Printf.printf "};\n";
;;
