#include <cstdio>
#include <cstdint>
#include <random>
#include <bit>
#include <iostream>
#include <cstdlib>
#include <intrin.h>
#include <immintrin.h>
#include <fstream>
#include <array>

using Uint64 = uint64_t;
using Uint8 = uint8_t;
using Uint16 = uint16_t;

int bsf(Uint64 x) {
    unsigned long res;
    _BitScanForward(&res, x);
    return (int)res;
}

int bsr(Uint64 x) {
    unsigned long res;
    _BitScanReverse(&res, x);
    return (int)res;
}

int const normal_rook_magic_ind[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 10,
    11, 10, 10, 10, 10, 10, 10, 10,
    11, 10, 10, 10, 10, 10, 10, 10,
    11, 10, 10, 10, 10, 10, 10, 10,
    11, 10, 10, 10, 10, 10, 10, 10,
    11, 10, 10, 10, 10, 10, 10, 10,
    12, 11, 11, 11, 11, 11, 11, 12,
};

int const normal_bishop_magic_ind[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6,
};

bool black_magic = false;

struct MagicBitBoardData {
    Uint8 postshift;
    int table_begin;
    Uint64 pre_mask;
    Uint64 post_mask;
    Uint64 magic;
};




Uint64 signed_right_shift(Uint64 x, int y) {
    return y >= 0 ? (x >> y) : (x << (-y));
}


Uint64 get_rook_mask(int sq) {
    Uint64 result = 0ULL;
    int rk = sq / 8, fl = sq % 8, r, f;
    for (r = rk + 1; r <= 6; r++) result |= (1ULL << (fl + r * 8));
    for (r = rk - 1; r >= 1; r--) result |= (1ULL << (fl + r * 8));
    for (f = fl + 1; f <= 6; f++) result |= (1ULL << (f + rk * 8));
    for (f = fl - 1; f >= 1; f--) result |= (1ULL << (f + rk * 8));
    return result;
}

Uint64 get_bishop_mask(int sq) {
    Uint64 result = 0ULL;
    int rk = sq / 8, fl = sq % 8, r, f;
    for (r = rk + 1, f = fl + 1; r <= 6 && f <= 6; r++, f++) result |= (1ULL << (f + r * 8));
    for (r = rk + 1, f = fl - 1; r <= 6 && f >= 1; r++, f--) result |= (1ULL << (f + r * 8));
    for (r = rk - 1, f = fl + 1; r >= 1 && f <= 6; r--, f++) result |= (1ULL << (f + r * 8));
    for (r = rk - 1, f = fl - 1; r >= 1 && f >= 1; r--, f--) result |= (1ULL << (f + r * 8));
    return result;
}


Uint64 get_bishop_accessible(int pos, Uint64 enemy_mask) {
    Uint64 result = 0ULL;
    int rank = pos / 8, file = pos % 8, r, f;
    for (r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++) {
        result |= (1ULL << (f + r * 8));
        if (enemy_mask & (1ULL << (f + r * 8))) break;
    }
    for (r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--) {
        result |= (1ULL << (f + r * 8));
        if (enemy_mask & (1ULL << (f + r * 8))) break;
    }
    for (r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++) {
        result |= (1ULL << (f + r * 8));
        if (enemy_mask & (1ULL << (f + r * 8))) break;
    }
    for (r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        result |= (1ULL << (f + r * 8));
        if (enemy_mask & (1ULL << (f + r * 8))) break;
    }
    return result;
}

Uint64 get_rook_accessible(int sq, Uint64 block) {
    Uint64 result = 0ULL;
    int rk = sq / 8, fl = sq % 8, r, f;
    for (r = rk + 1; r <= 7; r++) {
        result |= (1ULL << (fl + r * 8));
        if (block & (1ULL << (fl + r * 8))) break;
    }
    for (r = rk - 1; r >= 0; r--) {
        result |= (1ULL << (fl + r * 8));
        if (block & (1ULL << (fl + r * 8))) break;
    }
    for (f = fl + 1; f <= 7; f++) {
        result |= (1ULL << (f + rk * 8));
        if (block & (1ULL << (f + rk * 8))) break;
    }
    for (f = fl - 1; f >= 0; f--) {
        result |= (1ULL << (f + rk * 8));
        if (block & (1ULL << (f + rk * 8))) break;
    }
    return result;
}

Uint64 bishop_mask[64];
Uint64 rook_mask[64];


bool is_magic(Uint64 const magic_nb, int pos, Uint64 const postshift, bool is_bishop) {
    thread_local Uint64 accessible[1 << 12];
    

    memset(accessible, 0, sizeof(Uint64) * (1ull << (64 - postshift)));
    Uint64 const mask = (is_bishop ? bishop_mask : rook_mask)[pos];
    int const bits = (is_bishop ? normal_bishop_magic_ind : normal_rook_magic_ind)[pos];

    for (Uint16 nb = 0; nb < (1 << bits); nb++) {
        Uint64 board = black_magic * ~mask;
           
        Uint64 iter = mask;
        Uint16 x = nb;
        while (iter) {
            if (x & 1) {
                board |= (iter & ~(iter - 1));
            }
            iter &= iter - 1;
            x >>= 1;
        }
        Uint64 movement = is_bishop ? get_bishop_accessible(pos, board) : get_rook_accessible(pos, board);
        Uint64 hash = (board * magic_nb) >> postshift;

        // std::cout << board << ", " << movement << ", " << hash << '\n';

        if (accessible[hash] == 0) {
            accessible[hash] = movement;
        }
        else if (accessible[hash] != movement) {
            return false;
        }
    }

    return true;
}


int get_magic_table_length(int pos, bool is_bishop, MagicBitBoardData const magic) {
    int const bits = std::popcount(magic.pre_mask);
    int maxi = 0;
    for (Uint16 nb = 0; nb < (1 << bits); nb++) {
        Uint64 board = black_magic * ~magic.pre_mask;

        {
            Uint64 iter = magic.pre_mask;
            Uint16 x = nb;
            while (iter) {
                if (x & 1) {
                    board |= (iter & ~(iter - 1));
                }
                iter &= iter - 1;
                x >>= 1;
            }
        }
        Uint64 movement = is_bishop ? get_bishop_accessible(pos, board) : get_rook_accessible(pos, board);
        Uint64 hash = (board * magic.magic) >> magic.postshift;
        
        // std::cout << movement << " -> " << hash << "\n";
        if (hash > maxi)
            maxi = hash;
    }

    return maxi;
}


bool search_magic(int pos, Uint64 const postshift, bool is_bishop, Uint64 nb_of_try, Uint64* magic_nb, int id = -1) {
    bool success = false;

    thread_local std::random_device rd;
    thread_local std::mt19937_64 gen(rd());
    thread_local std::uniform_int_distribution<uint64_t> rand64;

    thread_local Uint64 accessible[1 << 12];

    /*for (; nb_of_try > 0; nb_of_try--) {
        

        //    Uint64 magic = 0xfffffcfcfd79edffull;

        Uint64 magic = rand64(gen);
        // Uint64 magic = 17653884425164982324ull;
        if (is_magic(magic, pos, postshift, is_bishop)) {
            *magic_nb = magic;
            success = true;
            break;
        }
     
    }
    std::cout << (success ? "success\n" : "failure\n");
    return success;*/
    Uint64 const mask = (is_bishop ? bishop_mask : rook_mask)[pos];
    int const bits = (is_bishop ? normal_bishop_magic_ind : normal_rook_magic_ind)[pos];

    Uint64 attack_patern[4096];
    Uint64 blocker_patern[4096];
    for (int i = 0; i < (1 << bits); i++) {
        blocker_patern[i] = _pdep_u64(i, mask);
        attack_patern[i] = is_bishop ? get_bishop_accessible(pos, blocker_patern[i]) : get_rook_accessible(pos, blocker_patern[i]);
    }
    for (; nb_of_try > 0; nb_of_try--) {
        Uint64 magic = rand64(gen) & rand64(gen) & rand64(gen);
        if (std::popcount((mask * magic) & 0xFF00000000000000ULL) < 6) continue;
        bool failed = false;
        memset(accessible, 0, sizeof(Uint64) * (1ull << (64 - postshift)));

        for (int i = 0, fail = 0; i < (1 << bits); i++) {
            Uint64 j = (blocker_patern[i] * magic) >> postshift;
            if (accessible[j] == 0ULL) 
                accessible[j] = attack_patern[i];
            else if (accessible[j] != attack_patern[i]) {
                failed = true;
                break;
            }
                
        }
        if (not failed) {
            *magic_nb = magic;
            return true;
        }
    }
}



//
/* current magic :

-a2 rook :
-magic = 140877074808832
- postshift = 53
- a2 bishop :
-magic = 5764626363177500800
- postshift = 59
- b3 rook :
-magic = 4629700554510053377
- postshift = 54
- c3 rook :
-magic = 145685559123968
- postshift = 54
- a3 rook :
-magic = 360435854564933634
- postshift = 53
- b3 bishop :
-magic = 45108598804129864
- postshift = 59
- c3 bishop :
-magic = 6345575177811665152
- postshift = 57
- a3 bishop :
-magic = 2936347026034671795
- postshift = 59
- b2 rook :
-magic = 18084835977338883
- postshift = 54
- b2 bishop :
-magic = 9079872383287496
- postshift = 59
- c2 rook :
-magic = 4644474555729024
- postshift = 54
- f3 rook :
-magic = 2305984296491418624
- postshift = 54
- c4 rook :
-magic = 1159757172693794832
- postshift = 54
- c2 bishop :
-magic = 2351446525320118785
- postshift = 59
- b4 rook :
-magic = 12970402113355005952
- postshift = 54
- c4 bishop :
-magic = 4611766283045765632
- postshift = 57
- b4 bishop :
-magic = 9241676844141529092
- postshift = 59
- f3 bishop :
-magic = 11533859404928516352
- postshift = 57
- d4 rook :
-magic = 18024298411200512
- postshift = 54
- f2 rook :
-magic = 3604146347881791616
- postshift = 54
- g4 rook :
-magic = 6936777095377854824
- postshift = 54
- g3 rook :
-magic = 6203712884884636161
- postshift = 54
- a4 rook :
-magic = 396352503484006412
- postshift = 53
- g4 bishop :
-magic = 598409237041168
- postshift = 59
- d4 bishop :
-magic = 298363613087926400
- postshift = 55
- g3 bishop :
-magic = 572849861566464
- postshift = 59
- f2 bishop :
-magic = 2242786918400
- postshift = 59
- a4 bishop :
-magic = 4522330048963088
- postshift = 59
- d2 rook :
-magic = 2305983781070180356
- postshift = 54
- g2 rook :
-magic = 9242793827712893440
- postshift = 54
- d2 bishop :
-magic = 648804289191609906
- postshift = 59
- b7 rook :
-magic = 1226176468957659648
- postshift = 54
- e4 rook :
-magic = 4618441978361937928
- postshift = 54
- g2 bishop :
-magic = 577596582209291392
- postshift = 59
- b7 bishop :
-magic = 9230692595356402697
- postshift = 59
- h4 rook :
-magic = 2305847415850172672
- postshift = 54
- c6 rook :
-magic = 144396800492634176
- postshift = 54
- h2 rook :
-magic = 4644341412823040
- postshift = 54
- h4 bishop :
-magic = 1180103081315149824
- postshift = 59
- h2 bishop :
-magic = 9948684815128593490
- postshift = 59
- c6 bishop :
-magic = 1179978372676194308
- postshift = 57
- c7 rook :
-magic = 144678413486037504
- postshift = 54
- f4 rook :
-magic = 562958543622152
- postshift = 54
- c7 bishop :
-magic = 581250792170192960
- postshift = 59
- f4 bishop :
-magic = 38289944833187846
- postshift = 57
- d3 rook :
-magic = 1180370262772682752
- postshift = 54
- b6 rook :
-magic = 4611773979894497281
- postshift = 54
- f7 rook :
-magic = 612491750493388928
- postshift = 54
- b6 bishop :
-magic = 720901400384652361
- postshift = 59
- d3 bishop :
-magic = 866943343035170816
- postshift = 57
- g7 rook :
-magic = 283984584057856
- postshift = 54
- e2 rook :
-magic = 141287311280128
- postshift = 54
- f7 bishop :
-magic = 144120273386405898
- postshift = 59
- g7 bishop :
-magic = 77128546027571224
- postshift = 59
- e2 bishop :
-magic = 2359887374047314008
- postshift = 59
- d6 rook :
-magic = 3459925641070247952
- postshift = 54
- a6 rook :
-magic = 4629736154286096384
- postshift = 53
- d6 bishop :
-magic = 721147969901955080
- postshift = 57
- a6 bishop :
-magic = 2597468823573497856
- postshift = 59
- c8 rook :
-magic = 20337684295714833
- postshift = 53
- c8 bishop :
-magic = 9340615167257384978
- postshift = 59
- f6 rook :
-magic = 288511868310585352
- postshift = 54
- g8 rook :
-magic = 4585032343650572
- postshift = 53
- g8 bishop :
-magic = 13835093256853520643
- postshift = 59
- e6 rook :
-magic = 281509337563140
- postshift = 54
- h3 rook :
-magic = 4857642380178697
- postshift = 54
- h3 bishop :
-magic = 70919104104456
- postshift = 59
- h6 rook :
-magic = 576461869003440128
- postshift = 54
- f6 bishop :
-magic = 288548139340666112
- postshift = 57
- h6 bishop :
-magic = 4689374221208650000
- postshift = 59
- e6 bishop :
-magic = 1161963890985140864
- postshift = 57
- g6 rook :
-magic = 54060792143937538
- postshift = 54
- g6 bishop :
-magic = 6926545096021114960
- postshift = 59
- e3 rook :
-magic = 81070291136022529
- postshift = 54
- b8 rook :
-magic = 8214711956444028949
- postshift = 53
- b8 bishop :
-magic = 566954627104
- postshift = 59
- e3 bishop :
-magic = 1154047413113323792
- postshift = 57
- b1 rook :
-magic = 3476814097775857670
- postshift = 53
- b1 bishop :
-magic = 4656801183850045440
- postshift = 59
- a7 rook :
-magic = 140738029424768
- postshift = 53
- a7 bishop :
-magic = 509052531769346049
- postshift = 59
- e4 bishop :
-magic = 180214628784078912
- postshift = 55
- d7 rook :
-magic = 1143492229529856
- postshift = 54
- d7 bishop :
-magic = 1166432304596385792
- postshift = 59
- h7 rook :
-magic = 504544995811009024
- postshift = 54
- h7 bishop :
-magic = 4616262188523716608
- postshift = 59
- d8 rook :
-magic = 18401446333513985
- postshift = 53
- a8 rook :
-magic = 2305913448439316481
- postshift = 52
- d8 bishop :
-magic = 1215972453484594180
- postshift = 59
- a8 bishop :
-magic = 281614867238913
- postshift = 58
- e7 rook :
-magic = 3386495881773312
- postshift = 54
- e7 bishop :
-magic = 37454295990290
- postshift = 59
- h8 rook :
-magic = 1153488870461997186
- postshift = 52
- h8 bishop :
-magic = 117137640722022416
- postshift = 58
- e8 rook :
-magic = 1166714070793619457
- postshift = 53
- e8 bishop :
-magic = 37159096054612480
- postshift = 59
- a1 rook :
-magic = 5800636389846453408
- postshift = 52
- a1 bishop :
-magic = 722996068685512800
- postshift = 58
- f8 rook :
-magic = 36873239129364483
- postshift = 53
- f8 bishop :
-magic = 281510444270084
- postshift = 59
- e1 rook :
-magic = 12249808717215175168
- postshift = 53
- e1 bishop :
-magic = 9368617660356493312
- postshift = 59
- a5 rook :
-magic = 70369289437312
- postshift = 53
- a5 bishop :
-magic = 144731533197287424
- postshift = 59
- e5 rook :
-magic = 1152965487261393920
- postshift = 54
- e5 bishop :
-magic = 282591669387296
- postshift = 55
- f1 rook :
-magic = 2449959365587894280
- postshift = 53
- f1 bishop :
-magic = 4789482314268672
- postshift = 59
- b5 rook :
-magic = 306315145561251844
- postshift = 54
- b5 bishop :
-magic = 36593001172047872
- postshift = 59
- f5 rook :
-magic = 563499751179264
- postshift = 54
- f5 bishop :
-magic = 144264730247184642
- postshift = 57
- d1 rook :
-magic = 7277834590690279520
- postshift = 53
- d1 bishop :
-magic = 2307040382225162778
- postshift = 59
- h1 rook :
-magic = 324269208361813248
- postshift = 52
- h1 bishop :
-magic = 6917670074401686016
- postshift = 58
- d5 rook :
-magic = 11529232642566260770
- postshift = 54
- c1 rook :
-magic = 10412339931203504256
- postshift = 53
- c1 bishop :
-magic = 18578044255996418
- postshift = 59
- d5 bishop :
-magic = 2269426901320704
- postshift = 55
- h5 rook :
-magic = 2388614811656126720
- postshift = 54
- h5 bishop :
-magic = 9800150552548606016
- postshift = 59
- g1 rook :
-magic = 11601312224689278464
- postshift = 53
- g1 bishop :
-magic = 9607541730590720
- postshift = 59
- c5 rook :
-magic = 76570006940295168
- postshift = 54
- c5 bishop :
-magic = 1153203288856922114
- postshift = 57
- g5 rook :
-magic = 585485964718141704
- postshift = 54
- g5 bishop :
-magic = 577872529954408464
- postshift = 59
*/
//

#include <thread>

void print_hex_padded16(Uint64 n) {
    putchar('0'); putchar('x');
    char arr[16] = { '0', '0', '0', '0', '0' };
    int i = 0;
    while (i < 16) {
        int digit = n & 0b1111;
        arr[i] = (digit < 10) ? '0' + digit : ('a' - 10 + digit);
        i++;
        n >>= 4;
    }
    while (i-- > 0) {
        putchar(arr[i]);
    }
}
struct MagicBitboardData {
    Uint64 pre_mask;
    Uint64 post_mask;
    Uint64 magic;
    int lookup_offset;
    int shift;
};

#include <utility>

std::pair<int, std::pair<Uint64, Uint64>> shared[] = {
    {
    (1 << 4),
    {
        0b00000001'00000010'00000000'00000000'00000000'00000000'00000000'00000000ull, // rooks
        0b00000000'00000000'00000000'00000000'00000000'01110000'00000000'00000000ull, // bishops
    }
    },
    {
    (1 << 4),
    {
        0b00000001'00000010'00000000'00000000'00000000'00000000'00000000'00000000ull, // rooks
        0b00000000'00000000'00000000'00000000'00000000'00110000'10000000'00000000ull, // bishops
    }
    },


    {
    (1 << 7) - (1 << 4) - (1 << 4),
    {
        0b00000001'00000010'00000000'00000000'00000000'00000000'00000000'00000000ull, // rooks
        0b00000000'00000000'00000000'00000000'00000000'00110000'00000000'00000000ull, // bishops
    }
    },







    {
    (1 << -1),
    {
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ull, // rooks
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ull, // bishops
    }
    },
};



int main()
{   

    for (int i = 0; i < 64; i++) {
        bishop_mask[i] = get_bishop_mask(i);
        rook_mask[i] = get_rook_mask(i);
    }
    // 0b0000000001000000001000000001000000001000000001000000001000000000
    // Uint64 const mask = 0b00000000'00000010'00000100'00001000'00010000'00100000'01000000'00000000ull;
    
    // Uint64 const mask = 0x0040201008040200ull;
    
    // MagicBitBoardData magic{ 56, -1, get_bishop_mask(27), 2722470505369436312ull };
    // std::cout << is_magic(magic.magic, 27, 55, true) << '\n';

    // std::cout << get_magic_table_length(0, false, magic) << '\n';

    
    // int const postshift = 64 - 9;
    bool const is_bishop = false;
    // int const pos = 18;
    std::thread threads[16];
    /*
    int i;
    for (i = 0; i < 16; i++) {
        threads[i] = std::thread([=]() -> void {
            int my_pos = 0;
            int postshift = 64 - 11;
            Uint64 magic_nb;
            if (search_magic(my_pos, postshift, false, -1, &magic_nb, -1)) {
                printf(" - %c%d rook :\n - magic = %llu\n - postshift = %d\n", 'a' + (my_pos & 0b111), 1 + (my_pos >> 3), magic_nb, postshift);
            }
        }
        );
    }*/
    
    /*int progress = 0;
    for (int my_pos = 0; my_pos < 64; my_pos++) {
        int postshift = 64 - normal_bishop_magic_ind[my_pos];
        Uint64 magic_nb;
        search_magic(my_pos, postshift, true, -1, &magic_nb, -1);
        printf("{ ");
        print_hex_padded16(get_bishop_mask(my_pos));
        printf(", ");
        print_hex_padded16(get_bishop_accessible(my_pos, 0));
        printf(", %llu, %u, %u }, \n", magic_nb, progress, postshift);
        progress += 1ull << (64 - postshift);
    }*/

    printf("let bishop_postmask = [|\n");
    for (int y = 0; y <= 7; y++) {
        printf("  [|");
        for (int x = 0; x <= 7; x++) {
            int pos = x | (y << 3);

            // Uint64 magic;
            // search_magic(pos, 64 - normal_bishop_magic_ind[pos], true, -1, &magic);
            printf("(Int64.of_string ");
            putchar('"');
            
            print_hex_padded16(get_bishop_accessible(pos, 0));
            
            putchar('"');
            printf("); ");
        }
        printf("|];\n");
    }
    printf("|];;\n");


    /*
    while (i --> 0) {
        threads[i].join();
    }*/

    return 0;
}