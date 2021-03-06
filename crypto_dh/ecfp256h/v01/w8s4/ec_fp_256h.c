/**
 * Scalar multiplication on twisted Hessian curves with a = 1.
 *
 *     a*X^3 + Y^3 + Z^3 = d*X Y Z.
 *
 * To guarantee a correct output, the base point should be of an order prime to 3.
 * In addition, the scalar must be smaller than the order of the base point.
 * The function ec_fp_smul_256h() is made public via finite.h.
 * The function ec_fp_smulbase_256h() is made public via finite.h.
 *
 * See EFD for a collection of algorithms for point operations.
 *
 * Huseyin Hisil.
 * 2009-2010.
 **/
#include <stdio.h>
#include "crympix.h"
#include "finite256.h"
#include "ec_fp_256h.h"

#define WINDOW_SIZE_LTR 5
#define TABLE_SIZE_LTR (1 << (WINDOW_SIZE_LTR - 2))

static void ec_fp_cpy_256H(uni X3, uni Y3, uni Z3, const uni X1, const uni Y1, const uni Z1){
	fp_cpy_256(X3, X1);
	fp_cpy_256(Y3, Y1);
	if(Z1 == NULL){
		fp_set_1_256(Z3, 1);
	}
	else{
		fp_cpy_256(Z3, Z1);
	}
}

static void ec_fp_neg_256H(uni X3, uni Y3, uni Z3, const uni X1, const uni Y1, const uni Z1){
	uni t;

	X3 = X1; t = Y1; Y3 = Z1; Z3 = t;
}

/*Cost: 4M+2S+4a.*/
static void ec_fp_mdbl_256H(uni X3, uni Y3, uni Z3, const uni X1, const uni Y1){
	uni_t t1[FP_LEN], t2[FP_LEN], t3[FP_LEN], t4[FP_LEN];

	fp_sqr_256(t2, Y1);
	fp_mul_256(t2, Y1, t2);
	fp_sqr_256(t3, X1);
	fp_mul_256(t3, X1, t3);
	fp_sub_1_256(t4, t2, 1);
	fp_mul_256(X3, X1, t4);
	fp_sub_256(t2, t3, t2);
	fp_sub_2_256(t1, 1, t3);
	fp_mul_256(Z3, Y1, t1);
	fp_cpy_256(Y3, t2);
}

/*Cost: 6M + 3S + 4a. */
inline static void ec_fp_dbl_256H(uni X3, uni Y3, uni Z3, const uni X1, const uni Y1, const uni Z1){
	uni_t t1[FP_LEN], t2[FP_LEN], t3[FP_LEN], t4[FP_LEN], t5[FP_LEN], t6[FP_LEN];

	fp_sqr_256(t2, Y1);
	fp_sqr_256(t1, Z1);
	fp_sqr_256(t3, X1);
	fp_mul_256(t1, Z1, t1);
	fp_mul_256(t5, t2, Y1);
	fp_mul_256(t3, t3, X1);
	fp_sub_256(t4, t5, t1);
	fp_sub_256(t1, t1, t3);
	fp_mul_256(X3, X1, t4);
	fp_sub_256(t2, t3, t5);
	fp_mul_256(t6, Z1, t2);
	fp_mul_256(Z3, Y1, t1);
	fp_cpy_256(Y3, t6);
}



/*Cost: 10M+3a. */
static void ec_fp_madd_256H(uni X3, uni Y3, uni Z3, const uni X1, const uni Y1, const uni Z1, const uni X2, const uni Y2){
	uni_t t1[FP_LEN], t2[FP_LEN], t3[FP_LEN], t4[FP_LEN], t5[FP_LEN], t6[FP_LEN];

	fp_mul_256(t1, Z1, X2);
	fp_mul_256(t2, Z1, Y2);
	fp_mul_256(t3, X1, Y2);
	fp_mul_256(t4, Y1, X2);
	fp_mul_256(t5, t3, X1);
	fp_mul_256(t6, t4, t1);
	fp_mul_256(t1, t2, t1);
	fp_mul_256(t2, t3, t2);
	fp_mul_256(t3, X1, Y1);
	fp_mul_256(t4, Y1, t4);
	fp_sub_256(X3, t5, t6);
	fp_sub_256(Y3, t1, t3);
	fp_sub_256(Z3, t4, t2);
}


/*Cost: 10M+3a. */
static void ec_fp_msub_256H(uni X3, uni Y3, uni Z3, const uni X1, const uni Y1, const uni Z1, const uni X2, const uni Y2){
	uni_t t1[FP_LEN], t2[FP_LEN], t3[FP_LEN], t4[FP_LEN], t5[FP_LEN], t6[FP_LEN];

	fp_mul_256(t1, X1, Y2);
	fp_mul_256(t3, Z1, X2);
	fp_mul_256(t2, Y1, Y2);
	fp_mul_256(t4, Y1, X2);
	fp_mul_256(t6, X1, t1);
	fp_mul_256(t5, t4, t3);
	fp_mul_256(t1, t2, t1);
	fp_mul_256(t3, Z1, t3);
	fp_mul_256(t2, t4, t2);
	fp_mul_256(t4, X1, Z1);
	fp_sub_256(X3, t6, t5);
	fp_sub_256(Y3, t3, t1);
	fp_sub_256(Z3, t2, t4);
}

/*Cost: 12M+3a. */
static void ec_fp_add_256H(uni X3, uni Y3, uni Z3, const uni X1, const uni Y1, const uni Z1, const uni X2, const uni Y2, const uni Z2){
	uni_t t1[FP_LEN], t2[FP_LEN], t3[FP_LEN], t4[FP_LEN], t5[FP_LEN], t6[FP_LEN];

	fp_mul_256(t1, X1, Z2);
	fp_mul_256(t3, Z1, X2);
	fp_mul_256(t2, Y1, Z2);
	fp_mul_256(t4, Z1, Y2);
	fp_mul_256(t5, X1, Y2);
	fp_mul_256(t6, Y1, X2);
	fp_mul_256(Y3, t3, t4);
	fp_mul_256(Z3, t6, t2);
	fp_mul_256(t3, t6, t3);
	fp_mul_256(X3, t5, t1);
	fp_sub_256(X3, X3, t3);
	fp_mul_256(t1, t2, t1);
	fp_mul_256(t4, t5, t4);
	fp_sub_256(Y3, Y3, t1);
	fp_sub_256(Z3, Z3, t4);
}

inline static void fp_cnt_256(int *bc, const uni an, const uni_t al){
	uni_t w, i, j;

	for(i = al - 1; (an[i] == 0) && (i > 0); i--);
	w = an[i];
	for(j = 0; w != 0; j++){
		w >>= 1;
	}
	(*bc) = j + i*UNIT_LEN;
}

inline static void find_nextwindow(int *v, int *k, int *wd, const uni en, const int i){
	int t, u2, s, b, sl, sr;
	uni_t n;

	b = i/UNIT_LEN;
	sr = (i + 1) - (b*UNIT_LEN);
	sl = UNIT_LEN - sr;
	if(b < FP_LEN){
		n = en[b] << sl;
	}
	else{
		n = 0;
	}
	if((b != 0) && (sl != 0)){
		n |= (en[b - 1] >> sr);
	}
	t = n >> (UNIT_LEN - 1);
	if(((n >> (UNIT_LEN - 2)) & 0x1) == t){
		*v = 0; *k = i; *wd = 1;
	}
	else{
		if(WINDOW_SIZE_LTR < (i + 1)){
			*wd = WINDOW_SIZE_LTR;
		}
		else{
			*wd = i + 1;
		}
		n <<= 1;
		n >>= (UNIT_LEN - *wd);
		if((i - *wd + 1) < 1){
			u2 = 0;
		}
		else{
			u2 = n & 0x1;
		}
		*v = -(t << (*wd - 1)) + (n >> 1) + u2;
		s = 0;
		for(; (*v & 0x1) == 0; s++, *v = *v >> 1);
		*k = i - (*wd - 1) + s + 1;
	}
}

void ec_fp_smul_256h(uni X1, uni Y1, uni Z1, const uni kn, const uni X2, const uni Y2)
{
	uni_t X[TABLE_SIZE_LTR][FP_LEN], Y[TABLE_SIZE_LTR][FP_LEN], Z[TABLE_SIZE_LTR][FP_LEN];
	int i, j, ni, k, wd;

	fp_cnt_256(&i, kn, FP_LEN);
	if(i == 0){
		fp_set_1_256(X1, 0);
		fp_set_1_256(Y1, 1);
		fp_set_1_256(Z1, 1);
		fp_sub_2_256(Y1, 0, Y1);
	}
	else{
		ec_fp_mdbl_256H(X[0], Y[0], Z[0], X2, Y2); /* 2P. */
		ec_fp_madd_256H(X[1], Y[1], Z[1], X[0], Y[0], Z[0], X2, Y2); /* 3P. */
		for(j = 2; j < TABLE_SIZE_LTR; j++){
			ec_fp_add_256H(X[j], Y[j], Z[j], X[j - 1], Y[j - 1], Z[j - 1], X[0], Y[0], Z[0]);
		}
		find_nextwindow(&ni, &k, &wd, kn, i);
		i -= wd;
		if(ni > 0){
			ni >>= 1;
			if(ni == 0){
				ec_fp_cpy_256H(X1, Y1, Z1, X2, Y2, NULL);
			}
			else{
				ec_fp_cpy_256H(X1, Y1, Z1, X[ni], Y[ni], Z[ni]);
			}
		}
		else{
			ni = (-ni) >> 1;
			if(ni == 0){
				ec_fp_neg_256H(X1, Y1, Z1, X2, Y2, NULL);
			}
			else{
				ec_fp_neg_256H(X1, Y1, Z1, X[ni], Y[ni], Z[ni]);
			}
		}
		for(j = k - 1; i >= 0; j--){
			find_nextwindow(&ni, &k, &wd, kn, i);
			i -= wd;
			if(ni == 0){
				ec_fp_dbl_256H(X1, Y1, Z1, X1, Y1, Z1); //DBL1++;
			}
			else{
				for(; j > k; j--){
					ec_fp_dbl_256H(X1, Y1, Z1, X1, Y1, Z1); //DBL2++;
				}
				ec_fp_dbl_256H(X1, Y1, Z1, X1, Y1, Z1); //DBL2++;
				if(ni > 0){
					ni >>= 1;
					if(ni == 0){
						ec_fp_madd_256H(X1, Y1, Z1, X1, Y1, Z1, X2, Y2); //DBL3++; ADD++;
					}
					else{
						ec_fp_add_256H(X1, Y1, Z1, X1, Y1, Z1, X[ni], Y[ni], Z[ni]); //DBL3++; ADD++;
					}
				}
				else{
					ni = (-ni) >> 1;
					if(ni == 0){
						ec_fp_msub_256H(X1, Y1, Z1, X1, Y1, Z1, X2, Y2); //ADD++;
					}
					else{
						ec_fp_add_256H(X1, Y1, Z1, X1, Y1, Z1, X[ni], Z[ni], Y[ni]); //ADD++;
					}
				}
			}
		}
		for(; j >= 1; j--){
			ec_fp_dbl_256H(X1, Y1, Z1, X1, Y1, Z1); //DBL4++;
		};
	}
}

#if EC_FP_SMULBASE_SLICE != 0
void ec_fp_smulbase_256h(uni X1, uni Y1, uni Z1, const uni kn, const uni *xn0, const uni *yn0)
{
	unsigned int i, j, k, m, n, p, ei, e[EC_FP_SMULBASE_SLICE];

	fp_set_1_256(X1, 0);
	fp_set_1_256(Y1, 1);
	fp_set_1_256(Z1, 1);
	fp_sub_2_256(Y1, 0, Y1);
	k = (UNIT_LEN*FP_LEN)/(EC_FP_SMULBASE_WINDOW*EC_FP_SMULBASE_SLICE);
	for(i = k; i > 0; i--){
		for(p = 0, m = 0; m < EC_FP_SMULBASE_SLICE; m++, p += k){
			e[m] = 0;
			for(n = 0, j = 0; j < EC_FP_SMULBASE_WINDOW; j++, n += ((UNIT_LEN*FP_LEN)/EC_FP_SMULBASE_WINDOW)){
				mam_ith_bit(ei, kn, i + n + p);
				e[m] |= (ei << j);
			}
			e[m] <<= 2;
		}
		ec_fp_dbl_256H(X1, Y1, Z1, X1, Y1, Z1);
		for(n = 0, j = 0; j < EC_FP_SMULBASE_SLICE; j++, n += ((EC_FP_SMULBASE_CACHE_SIZE*sizeof(unsigned char))/sizeof(uni_t))){
			if(e[j] != 0){
				m = e[j] + n;
				ec_fp_madd_256H(X1, Y1, Z1, X1, Y1, Z1, (uni)(xn0 + m), (uni)(yn0 + m));
			}
		}
	}
}
#endif





///*Cost: 7M + 1S + 8a. FIX cpy.*/
//static void ec_fp_dbl_256H(uni X3, uni Y3, uni Z3, const uni X1, const uni Y1, const uni Z1){
//	uni_t t1[FP_LEN], t2[FP_LEN], t3[FP_LEN];
//
//	fp_sqr_256(t1, Y1);
//	fp_add_256(t2, Y1, Z1);
//	fp_mul_256(t2, t2, Z1);
//	fp_add_256(t2, t1, t2);
//	fp_sub_256(t3, Z1, Y1);
//	fp_mul_256(t2, t3, t2);
//	fp_add_256(t3, X1, Y1);
//	fp_mul_256(t3, X1, t3);
//	fp_add_256(t3, t3, t1);
//	fp_sub_256(t1, Y1, X1);
//	fp_mul_256(t1, t1, t3);
//	fp_mul_256(X3, X1, t2);
//	fp_mul_256(t3, Z1, t1);
//	fp_sub_2_256(t1, 0, t1);
//	fp_sub_256(t1, t1, t2);
//	fp_mul_256(Z3, Y1, t1);
//	fp_cpy_256(Y3, t3);
//}


