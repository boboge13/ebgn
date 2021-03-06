/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 312230917
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/
#ifndef _SEAFP_CON_TBL_H
#define _SEAFP_CON_TBL_H

#include "type.h"
#if 0
static UINT32 oddsmallprimetable[ SEAFP_SMALL_PRIME_NUM ] = {
    3 ,5 ,7 ,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97
};
#endif
static UINT32 stable[ SEAFP_SMALL_PRIME_NUM ]= {
    6 ,3 ,2 ,6 ,1 ,3 ,2 ,6 ,3 ,2 ,1 ,3 ,2 ,6 ,3 ,6 ,1 ,2 ,6 ,1 ,2 ,6 ,3 ,1
};

#if 0
static UINT32 pmododdsmallprimetable[SEAFP_SMALL_PRIME_NUM]={
    2 ,4 ,5 ,9 ,10,16,12,2 ,24,18,13,20,36,42,20,1 ,53,4 ,66,5 ,37,56,53,36
};
#endif
static INT32 oddfactor[SEAFP_SMALL_PRIME_NUM][SEAFP_NUM_OF_FACTOR]={
    /* 0*/{4,-1},            //3
    /* 1*/{2,6,-1},            //5
    /* 2*/{8,-1},            //7
    /* 3*/{4,12,-1},        //11
    /* 4*/{2,14,-1},        //13
    /* 5*/{2,6,18,-1},        //17
    /* 6*/{4,20,-1},        //19
    /* 7*/{8,24,-1},        //23
    /* 8*/{2,6,10,30,-1},        //29
    /* 9*/{32,-1},            //31
    /*10*/{2,38,-1},        //37
    /*11*/{2,6,14,42,-1},        //41
    /*12*/{4,44,-1},        //43
    /*13*/{16,48,-1},        //47
    /*14*/{2,6,18,54,-1},        //53
    /*15*/{4,12,20,60,-1},        //59
    /*16*/{2,62,-1},        //61
    /*17*/{4,68,-1},        //67
    /*18*/{8,24,72,-1},        //71
    /*19*/{2,74,-1},        //73
    /*20*/{16,80,-1},        //79
    /*21*/{4,12,28,84,-1},        //83
    /*22*/{2,6,10,18,30,90,-1},    //89
    /*23*/{2,14,98,-1}         //97
};
static INT32 evenfactor[SEAFP_SMALL_PRIME_NUM][SEAFP_NUM_OF_FACTOR]={
    /*0 */{2,-1},            //3
    /*1 */{3,-1},            //5
    /*2 */{2,4,-1},            //7
    /*3 */{2,3,6,-1},        //11
    /*4 */{7,-1},            //13
    /*5 */{3,9,-1},            //17
    /*6 */{2,5,10,-1},        //19
    /*7 */{2,3,4,6,12,-1},        //23
    /*8 */{3,5,15,-1},        //29
    /*9 */{2,4,8,16,-1},        //31
    /*10*/{19,-1},            //37
    /*11*/{3,7,21,-1},        //41
    /*12*/{2,11,22,-1},        //43
    /*13*/{2,3,4,6,8,12,24,-1},    //47
    /*14*/{3,9,27,-1},        //53
    /*15*/{2,3,5,6,10,15,30,-1},    //59
    /*16*/{31,-1},            //61
    /*17*/{2,17,34,-1},        //67
    /*18*/{2,3,4,6,9,12,18,36,-1},    //71
    /*19*/{37,-1},            //73
    /*20*/{2,4,5,8,10,20,40,-1},    //79
    /*21*/{2,3,6,7,14,21,42,-1},    //83
    /*22*/{3,5,9,15,45,-1},        //89
    /*23*/{7,49,-1}            //97
};

static INT32 legendretable[SEAFP_SMALL_PRIME_NUM][SEAFP_MAX_INDEX]={
// if legendretable[lpos][i]>0 then legendretable[lpos][i]^2 = i mod ltable[lpos]
// legendretable[lpos][i] <---> Legendre symbol :(i/l)
/*0 : 3*/ {0, 1, -1},
/*1 : 5*/ {0, 1, -1, -1, 2},
/*2 : 7*/ {0, 1, 3, -1, 2, -1, -1},
/*3 :11*/ {0, 1, -1, 5, 2, 4, -1, -1, -1, 3, -1},
/*4 :13*/ {0, 1, -1, 4, 2, -1, -1, -1, -1, 3, 6, -1, 5},
/*5 :17*/ {0, 1, 6, -1, 2, -1, -1, -1, 5, 3, -1, -1, -1, 8, -1, 7, 4},
/*6 :19*/ {0, 1, -1, -1, 2, 9, 5, 8, -1, 3, -1, 7, -1, -1, -1, -1, 4, 6, -1},
/*7 :23*/ {0, 1, 5, 7, 2, -1, 11, -1, 10, 3, -1, -1, 9, 6, -1, -1, 4, -1, 8, -1, -1, -1, -1},
/*8 :29*/ {0, 1, -1, -1, 2, 11, 8, 6, -1, 3, -1, -1, -1, 10, -1, -1, 4, -1, -1, -1, 7, -1,
           14, 9, 13, 5, -1, -1, 12},
/*9 :31*/ {0, 1, 8, -1, 2, 6, -1, 10, 15, 3, 14, -1, -1, -1, 13, -1, 4, -1, 7, 9, 12, -1,
           -1, -1, -1, 5, -1, -1, 11, -1, -1},
/*10:37*/ {0, 1, -1, 15, 2, -1, -1, 9, -1, 3, 11, 14, 7, -1, -1, -1, 4, -1, -1, -1, -1, 13,
           -1, -1, -1, 5, 10, 8, 18, -1, 17, -1, -1, 12, 16, -1, 6},
/*11:41*/ {0, 1, 17, -1, 2, 13, -1, -1, 7, 3, 16, -1, -1, -1, -1, -1, 4, -1, 10, -1, 15, 12,
           -1, 8, -1, 5, -1, -1, -1, -1, -1, 20, 14, 19, -1, -1, 6, 18, -1,11, 9},
/*12:43*/ {0, 1, -1, -1, 2, -1, 7, -1, -1, 3, 15, 21, -1, 20, 10, 12, 4, 19, -1, -1, -1, 8,
           -1, 18, 14, 5, -1, -1, -1, -1, -1, 17, -1, -1, -1, 11, 6, -1, 9, -1, 13, 16, -1},
/*13:47*/ {0, 1, 7, 12, 2, -1, 10, 17, 14, 3, -1, -1, 23, -1, 22, -1, 4, 8, 21, -1, -1, 16,
           -1, -1, 20, 5, -1, 11, 13, -1, -1, -1, 19, -1, 9, -1, 6, 15, -1, -1, -1, -1, 18,
           -1, -1, -1, -1},
/*14:53*/ {0, 1, -1, -1, 2, -1, 18, 22, -1, 3, 13, 8, -1, 15, -1, 11, 4, 21, -1, -1, -1, -1,
           -1, -1, 17, 5, -1, -1, 9, 20, -1, -1, -1, -1, -1, -1, 6, 14, 12,-1, 26, -1, 25,
           19, 16, -1, 24, 10, -1, 7, -1, -1, 23},
/*15:59*/ {0, 1, -1, 11, 2, 8, -1, 19, -1, 3, -1, -1, 22, -1, -1, 29, 4, 28, -1, 14, 16, 27,
           9, -1, -1, 5, 12, 26, 21, 18, -1, -1, -1, -1, -1, 25, 6, -1, -1, -1, -1, 10, -1,
           -1, -1, 24, 20, -1, 15, 7, -1, 13, -1, 17, -1, -1, -1, 23, -1},
/*16:61*/ {0, 1, -1, 8, 2, 26, -1, -1, -1, 3, -1, -1, 16, 14, 21, 25, 4, -1, -1, 18, 9, -1,
           12, -1, -1, 5, -1, 24, -1, -1, -1, -1, -1, -1, 20, -1, 6, -1, -1, 10, -1, 23, 15,
           -1, -1, 17, 30, 13, 29, 7, -1, -1, 28, -1, -1, -1, 19, 22, 27, -1, 11},
/*17:67*/ {0, 1, -1, -1, 2, -1, 26, -1, -1, 3, 12, -1, -1, -1, 9, 22, 4, 33, -1, 32, -1, 17,
           25, 31, 15, 5, 19, -1, -1, 30, -1, -1, -1, 10, -1, 13, 6, 29, -1, 21, 24, -1, -1,
           -1, -1, -1, -1, 28, -1, 7, -1, -1, -1, -1, 11, 16, 18, -1, -1, 27, 23, -1, 14, -1,
           8, 20, -1},
/*18:71*/ {0, 1, 12, 28, 2, 17, 19, -1, 24, 3, 9, -1, 15, -1, -1, 21, 4, -1, 35, 27, 34, -1,
            -1, -1, 33, 5, -1, 13, -1, 10, 32, -1, 23, -1, -1, -1, 6, 26, 31, -1, 18, -1, -1,
            16, -1, 20, -1, -1, 30, 7, 11, -1, -1, -1, 14, -1, -1, 25, 22, -1, 29, -1, -1, -1,
            8, -1, -1, -1, -1, -1, -1},
/*19:73*/ {0, 1, 32, 21, 2, -1, 15, -1, 9, 3, -1, -1, 31, -1, -1, -1, 4, -1, 23, 26, -1, -1, -1,
           13, 30, 5, -1, 10, -1, -1, -1, -1, 18, -1, -1, 20, 6, 16, 29, -1, -1, 25, -1, -1, -1,
           -1, 22, -1, 11, 7, 14, -1, -1, -1, 28, 36, -1, 35, -1, -1, -1, 34, -1, -1, 8, 24, -1,
           33, -1, 19, 17, 12, 27},
/*20:79*/ {0, 1, 9, -1, 2, 20, -1, -1, 18, 3, 22, 13, -1, 31, -1, -1, 4, -1, 27, 16, 39, 10, 38,
           24, -1, 5, 37, -1, -1, -1, -1, 30, 36, -1, -1, -1, 6, -1, 14, -1, 35, -1, 11, -1, 26,
           19, 21, -1, -1, 7, 34, 29, 17, -1, -1, 23, -1, -1, -1, -1, -1, -1, 33, -1, 8, 12, -1,
           15, -1, -1, -1, -1, 25, 28, -1, -1, 32, -1, -1},
/*21:83*/ {0, 1, -1, 13, 2, -1, -1, 16, -1, 3, 33, 29, 26, -1, -1, -1, 4, 10, -1, -1, -1, 41, -1,
            40, -1, 5, 21, 39, 32, 19, 14, 23, -1, 38, -1, -1, 6, 28, 11, -1, 17, 37, -1, -1, 25,
            -1, -1, -1, 31, 7, -1, 36, -1, -1, -1, -1, -1, -1, -1, 15, -1, 12, -1, 35, 8, 27, -1,
            -1, 20, 22, 30, -1, -1, -1, -1, 18, -1, 34, 24, -1, -1, 9, -1},
/*22:89*/ {0, 1, 25, -1, 2, 19, -1, -1, 39, 3, 30, 10, -1, -1, -1, -1, 4, 27, 14, -1, 38, 33, 17,
           -1, -1, 5, -1, -1, -1, -1, -1, -1, 11, -1, 37, -1, 6, -1, -1, 22, 29, -1, 24, -1, 20,
           32, -1, 15, -1, 7, 36, -1, -1, 26, -1, 12, -1, 18, -1, -1, -1, -1, -1, -1, 8, -1, -1,
           44, 35, 43, -1, 31, 28, 42, -1, -1, -1, -1, 16, 41, 13, 9, -1, -1, 23, 21, -1, 40, 34},
/*23:97*/ {0, 1, 14, 10, 2, -1, 43, -1, 28, 3, -1, 37, 20, -1, -1, -1, 4, -1, 42, -1, -1, -1, 33,
          -1, 11, 5, -1, 30, -1, -1, -1, 15, 41, 18, -1, 36, 6, -1, -1, -1, -1, -1, -1, 25, 23,
          -1, -1, 12, 40, 7, 27, -1, -1, 21, 32, -1, -1, -1, -1, -1, -1, 35, 16, -1, 8, 29, 39,
          -1, -1, -1, 19, -1, 13, 48, -1, 47, -1, -1, -1, 46, -1, 9, -1, -1, -1, 45, 38, -1, 31,
          34, -1, 24, -1, 44, 26, 17, 22}
};

static INT32 factortable[SEAFP_SMALL_PRIME_NUM][SEAFP_MAX_FACTOR]={
    /* 0*/{2,4,-1},                 //3
    /* 1*/{2,3,6,-1},                 //5
    /* 2*/{2,4,8,-1},                 //7
    /* 3*/{2,3,4,6,12,-1},             //11
    /* 4*/{2,7,14,-1},                 //13
    /* 5*/{2,3,6,9,18,-1},             //17
    /* 6*/{2,4,5,10,20,-1},             //19
    /* 7*/{2,3,4,6,8,12,24,-1},         //23
    /* 8*/{2,3,5,6,10,15,30,-1},         //29
    /* 9*/{2,4,8,16,32,-1},             //31
    /*10*/{2,19,38,-1},                   //37
    /*11*/{2,3,6,7,14,21,42,-1},         //41
    /*12*/{2,4,11,22,44,-1},            //43
    /*13*/{2,3,4,6,8,12,16,24,48,-1},          //47
    /*14*/{2,3,6,9,18,27,54,-1},               //53
    /*15*/{2,3,4,5,6,10,12,15,20,30,60,-1}, //59
    /*16*/{2,31,62,-1},                     //61
    /*17*/{2,4,17,34,68,-1},                //67
    /*18*/{2,3,4,6,8,9,12,18,24,36,72,-1},  //71
    /*19*/{2,37,74,-1},                     //73
    /*20*/{2,4,5,8,10,16,20,40,80,-1},      //79
    /*21*/{2,3,4,6,7,12,14,21,28,42,84,-1}, //83
    /*22*/{2,3,5,6,9,10,15,18,30,45,90,-1}, //89
    /*23*/{2,7,14,49,98,-1}                 //97
};

static INT32 fl2rztable[SEAFP_SMALL_PRIME_NUM][SEAFP_MAX_FACTOR][SEAFP_MAX_LEN_OF_ZTABLE]={
    /* l =  3,lpos =  0 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  4*/{2,-1}, /*len =  1 */
    },
    /* l =  5,lpos =  1 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{2,-1}, /*len =  1 */
        /* r =  6*/{0,-1}, /*len =  0 */
    },
    /* l =  7,lpos =  2 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  4*/{0,-1}, /*len =  0 */
        /* r =  8*/{2, 4,-1}, /*len =  2 */
    },
    /* l = 11,lpos =  3 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{8,-1}, /*len =  1 */
        /* r =  4*/{0,-1}, /*len =  0 */
        /* r =  6*/{4,-1}, /*len =  1 */
        /* r = 12*/{0,-1}, /*len =  0 */
    },
    /* l = 13,lpos =  4 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  7*/{4, 8, 10,-1}, /*len =  3 */
        /* r = 14*/{0,-1}, /*len =  0 */
    },
    /* l = 17,lpos =  5 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{4,-1}, /*len =  1 */
        /* r =  6*/{0,-1}, /*len =  0 */
    //    /* r =  9*/{6, 12, 16,-1}, /*len =  3 */
    {6, 5, 16,-1},//for test !!!!!!!!!!!!
        /* r = 18*/{0,-1}, /*len =  0 */
    },
    /* l = 19,lpos =  6 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  4*/{10,-1}, /*len =  1 */
        /* r =  5*/{0,-1}, /*len =  0 */
        /* r = 10*/{0,-1}, /*len =  0 */
        ///* r = 20*/{2, 14, 16, 18,-1}, /*len =  4 */
        /* r = 20*/{2, 5, 16, 18,-1}, /*len =  4 *///for test
    },
    /* l = 23,lpos =  7 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{18,-1}, /*len =  1 */
        /* r =  4*/{2,-1}, /*len =  1 */
        /* r =  6*/{12,-1}, /*len =  1 */
        /* r =  8*/{0,-1}, /*len =  0 */
        /* r = 12*/{6, 8,-1}, /*len =  2 */
        /* r = 24*/{0,-1}, /*len =  0 */
    },
    /* l = 29,lpos =  8 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{16,-1}, /*len =  1 */
        /* r =  5*/{20, 22,-1}, /*len =  2 */
        /* r =  6*/{0,-1}, /*len =  0 */
        /* r = 10*/{0,-1}, /*len =  0 */
    //    /* r = 15*/{6, 8, 12, 28,-1}, /*len =  4 */
    {23,21,17,1,-1},// for test !!!!!
        /* r = 30*/{0,-1}, /*len =  0 */
    },
    /* l = 31,lpos =  9 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  4*/{6,-1}, /*len =  1 */
        /* r =  8*/{4, 26,-1}, /*len =  2 */
        /* r = 16*/{8, 16, 28, 30,-1}, /*len =  4 */
        /* r = 32*/{0,-1}, /*len =  0 */
    },
    /* l = 37,lpos = 10 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r = 19*/{0,-1}, /*len =  0 */
        /* r = 38*/{12, 16, 18, 20, 24, 26, 28, 34, 36,-1}, /*len =  9 */
    },
    /* l = 41,lpos = 11 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{26,-1}, /*len =  1 */
        /* r =  6*/{0,-1}, /*len =  0 */
        /* r =  7*/{22, 36, 40,-1}, /*len =  3 */
        /* r = 14*/{0,-1}, /*len =  0 */
        /* r = 21*/{2, 6, 14, 16, 28, 38,-1}, /*len =  6 */
        /* r = 42*/{0,-1}, /*len =  0 */
    },
    /* l = 43,lpos = 12 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  4*/{0,-1}, /*len =  0 */
        /* r = 11*/{2, 24, 32, 40, 42,-1}, /*len =  5 */
        /* r = 22*/{10, 16, 18, 22, 36,-1}, /*len =  5 */
        /* r = 44*/{0,-1}, /*len =  0 */
    },
    /* l = 47,lpos = 13 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{18,-1}, /*len =  1 */
        /* r =  4*/{32,-1}, /*len =  1 */
        /* r =  6*/{28,-1}, /*len =  1 */
        /* r =  8*/{40, 42,-1}, /*len =  2 */
        /* r = 12*/{12, 20,-1}, /*len =  2 */
        /* r = 16*/{0,-1}, /*len =  0 */
        /* r = 24*/{10, 16, 26, 44,-1}, /*len =  4 */
        /* r = 48*/{0,-1}, /*len =  0 */
    },
    /* l = 53,lpos = 14 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{0,-1}, /*len =  0 */
        /* r =  6*/{22,-1}, /*len =  1 */
        /* r =  9*/{0,-1}, /*len =  0 */
        /* r = 18*/{20, 38, 48,-1}, /*len =  3 */
        /* r = 27*/{0,-1}, /*len =  0 */
        /* r = 54*/{2, 10, 18, 24, 36, 42, 46, 50, 52,-1}, /*len =  9 */
    },
    /* l = 59,lpos = 15 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{58,-1}, /*len =  1 */
        /* r =  4*/{0,-1}, /*len =  0 */
        /* r =  5*/{26, 34,-1}, /*len =  2 */
        /* r =  6*/{48,-1}, /*len =  1 */
        /* r = 10*/{6, 38,-1}, /*len =  2 */
        /* r = 12*/{0,-1}, /*len =  0 */
        /* r = 15*/{10, 20, 44, 46,-1}, /*len =  4 */
        /* r = 20*/{0,-1}, /*len =  0 */
        /* r = 30*/{22, 28, 30, 50,-1}, /*len =  4 */
        /* r = 60*/{0,-1}, /*len =  0 */
    },
    /* l = 61,lpos = 16 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r = 31*/{0,-1}, /*len =  0 */
        /* r = 62*/{6, 8, 10, 12, 16, 18, 22, 24, 26, 28, 30, 34, 48, 50, 60,-1}, /*len = 15 */
    },
    /* l = 67,lpos = 17 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  4*/{0,-1}, /*len =  0 */
        /* r = 17*/{8, 14, 26, 28, 30, 40, 44, 66,-1}, /*len =  8 */
        /* r = 34*/{6, 12, 22, 32, 36, 50, 52, 56,-1}, /*len =  8 */
        /* r = 68*/{0,-1}, /*len =  0 */
    },
    /* l = 71,lpos = 18 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{0,-1}, /*len =  0 */
        /* r =  4*/{0,-1}, /*len =  0 */
        /* r =  6*/{0,-1}, /*len =  0 */
        /* r =  8*/{60, 70,-1}, /*len =  2 */
        /* r =  9*/{0,-1}, /*len =  0 */
        /* r = 12*/{0,-1}, /*len =  0 */
        /* r = 18*/{0,-1}, /*len =  0 */
        /* r = 24*/{16, 24, 44, 48,-1}, /*len =  4 */
        /* r = 36*/{0,-1}, /*len =  0 */
    //    /* r = 72*/{6, 8, 12, 20, 22, 28, 30, 38, 50, 52, 58, 64,-1}, /*len = 12 */
    //{6, 8, 12, 20, 22, 28, 30, 38, 21, 52, 58, 64,-1},//for test !!!!!
    {21,6, 8, 12, 20, 22, 28, 30, 38, 52, 58, 64,-1}
    },
    /* l = 73,lpos = 19 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r = 37*/{0,-1}, /*len =  0 */
        /* r = 74*/{8, 10, 12, 14, 16, 20, 22, 24, 28, 40, 46, 48, 52, 58, 62, 66, 68, 70,-1}, /*len = 18 */
    },
    /* l = 79,lpos = 20 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  4*/{0,-1}, /*len =  0 */
        /* r =  5*/{0,-1}, /*len =  0 */
        /* r =  8*/{0,-1}, /*len =  0 */
        /* r = 10*/{0,-1}, /*len =  0 */
        /* r = 16*/{50, 52, 64, 70,-1}, /*len =  4 */
        /* r = 20*/{0,-1}, /*len =  0 */
        /* r = 40*/{0,-1}, /*len =  0 */
    //    /* r = 80*/{2, 8, 12, 14, 16, 20, 24, 26, 30, 32, 34, 40, 54, 58, 72, 74,-1}, /*len = 16 */
    {7,2, 8, 12, 14, 16, 20, 24, 26, 30, 32, 34, 40, 54, 58, 74,-1},//for test !!!!
    },
    /* l = 83,lpos = 21 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{0,-1}, /*len =  0 */
        /* r =  4*/{64,-1}, /*len =  1 */
        /* r =  6*/{0,-1}, /*len =  0 */
        /* r =  7*/{0,-1}, /*len =  0 */
        /* r = 12*/{50, 52,-1}, /*len =  2 */
        /* r = 14*/{0,-1}, /*len =  0 */
        /* r = 21*/{0,-1}, /*len =  0 */
        /* r = 28*/{10, 24, 46, 48, 76, 80,-1}, /*len =  6 */
        /* r = 42*/{0,-1}, /*len =  0 */
        ///* r = 84*/{8, 14, 16, 28, 32, 34, 36, 38, 42, 44, 60, 78,-1}, /*len = 12 */
    {5,8, 14, 16, 28, 32, 34, 36, 38, 42, 44, 60, -1}, //for test !!!
    },
    /* l = 89,lpos = 22 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        /* r =  3*/{26,-1}, /*len =  1 */
        /* r =  5*/{56, 82,-1}, /*len =  2 */
        /* r =  6*/{0,-1}, /*len =  0 */
        /* r =  9*/{2, 44, 46,-1}, /*len =  3 */
        /* r = 10*/{0,-1}, /*len =  0 */
        /* r = 15*/{38, 50, 58, 72,-1}, /*len =  4 */
        /* r = 18*/{0,-1}, /*len =  0 */
        /* r = 30*/{0,-1}, /*len =  0 */
        /* r = 45*/{8, 10, 18, 28, 30, 34, 60, 68, 70, 74, 76, 88,-1}, /*len = 12 */
        /* r = 90*/{0,-1}, /*len =  0 */
    },
    /* l = 97,lpos = 23 */{
        /* r =  2*/{0,-1}, /*len =  0 */
        ///* r =  7*/{14, 44, 52,-1}, /*len =  3 */
    {83,53,45,-1},// for test !!!!!!!!!!!!!!!!
        /* r = 14*/{0,-1}, /*len =  0 */
        /* r = 49*/{8, 16, 18, 28, 30, 32, 34, 38, 42, 48, 50, 54, 56, 58, 62, 66, 78, 86, 88, 94, 96,-1}, /*len = 21 */
        /* r = 98*/{0,-1}, /*len =  0 */
    },
};/* maxlen = 11 */
#if 0
static INT32 squrootof4pmodl[ SEAFP_SMALL_PRIME_NUM ]={
    -1,1,-1,5,1,8,-1,10,3,14,-1,11,12,11,-1,2,-1,4,-1,-1,-1,-1,37,12
};
#endif
static UINT32 _oddsmallprime_tbl[ ] = {
                3     ,   5     ,   7    ,   11    ,   13   ,    17   ,    19  ,
      23    ,   29    ,   31    ,   37   ,    41   ,    43  ,     47  ,     53 ,
      59    ,   61    ,   67    ,   71   ,    73   ,    79  ,     83  ,     89 ,
      97    ,  101    ,  103    ,  107   ,   109   ,   113  ,    127  ,    131 ,
     137    ,  139    ,  149    ,  151   ,   157   ,   163  ,    167  ,    173 ,
     179    ,  181    ,  191    ,  193   ,   197   ,   199  ,    211  ,    223 ,
     227    ,  229    ,  233    ,  239   ,   241   ,   251  ,    257  ,    263 ,
     269    ,  271    ,  277    ,  281   ,   283   ,   293  ,    307  ,    311 ,
     313    ,  317    ,  331    ,  337   ,   347   ,   349  ,    353  ,    359 ,
     367    ,  373    ,  379    ,  383   ,   389   ,   397  ,    401  ,    409 ,
};
static UINT32 _oddsmallprime_tbl_size = sizeof(_oddsmallprime_tbl)/sizeof(_oddsmallprime_tbl[ 0 ]);

#endif /*_SEAFP_CON_TBL_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

