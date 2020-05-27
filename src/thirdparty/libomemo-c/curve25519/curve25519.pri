SOURCES += $$PWD/curve25519-donna.c \
    $$PWD/ed25519/fe_0.c \
    $$PWD/ed25519/fe_1.c \
    $$PWD/ed25519/fe_add.c \
    $$PWD/ed25519/fe_cmov.c \
    $$PWD/ed25519/fe_copy.c \
    $$PWD/ed25519/fe_frombytes.c \
    $$PWD/ed25519/fe_invert.c \
    $$PWD/ed25519/fe_isnegative.c \
    $$PWD/ed25519/fe_isnonzero.c \
    $$PWD/ed25519/fe_mul.c \
    $$PWD/ed25519/fe_neg.c \
    $$PWD/ed25519/fe_pow22523.c \
    $$PWD/ed25519/fe_sq.c \
    $$PWD/ed25519/fe_sq2.c \
    $$PWD/ed25519/fe_sub.c \
    $$PWD/ed25519/fe_tobytes.c \
    $$PWD/ed25519/ge_add.c \
    $$PWD/ed25519/ge_double_scalarmult.c \
    $$PWD/ed25519/ge_frombytes.c \
    $$PWD/ed25519/ge_madd.c \
    $$PWD/ed25519/ge_msub.c \
    $$PWD/ed25519/ge_p1p1_to_p2.c \
    $$PWD/ed25519/ge_p1p1_to_p3.c \
    $$PWD/ed25519/ge_p2_0.c \
    $$PWD/ed25519/ge_p2_dbl.c \
    $$PWD/ed25519/ge_p3_0.c \
    $$PWD/ed25519/ge_p3_dbl.c \
    $$PWD/ed25519/ge_p3_to_cached.c \
    $$PWD/ed25519/ge_p3_to_p2.c \
    $$PWD/ed25519/ge_p3_tobytes.c \
    $$PWD/ed25519/ge_precomp_0.c \
    $$PWD/ed25519/ge_scalarmult_base.c \
    $$PWD/ed25519/ge_sub.c \
    $$PWD/ed25519/ge_tobytes.c \
    $$PWD/ed25519/open.c \
    $$PWD/ed25519/sc_muladd.c \
    $$PWD/ed25519/sc_reduce.c \
    $$PWD/ed25519/sign.c \
    $$PWD/ed25519/additions/compare.c \
    $$PWD/ed25519/additions/curve_sigs.c \
	$$PWD/ed25519/additions/ed_sigs.c \
    $$PWD/ed25519/additions/elligator.c \
    $$PWD/ed25519/additions/fe_isequal.c \
    $$PWD/ed25519/additions/fe_isreduced.c \
	$$PWD/ed25519/additions/fe_edy_to_montx.c \
    $$PWD/ed25519/additions/fe_mont_rhs.c \
    $$PWD/ed25519/additions/fe_montx_to_edy.c \
    $$PWD/ed25519/additions/fe_sqrt.c \
    $$PWD/ed25519/additions/ge_isneutral.c \
    $$PWD/ed25519/additions/ge_montx_to_p3.c \
    $$PWD/ed25519/additions/ge_neg.c \
    $$PWD/ed25519/additions/ge_p3_to_montx.c \
    $$PWD/ed25519/additions/ge_scalarmult.c \
    $$PWD/ed25519/additions/ge_scalarmult_cofactor.c \
    $$PWD/ed25519/additions/keygen.c \
    $$PWD/ed25519/additions/open_modified.c \
    $$PWD/ed25519/additions/sc_clamp.c \
    $$PWD/ed25519/additions/sc_cmov.c \
    $$PWD/ed25519/additions/sc_neg.c \
    $$PWD/ed25519/additions/sign_modified.c \
    $$PWD/ed25519/additions/utility.c \
    $$PWD/ed25519/additions/generalized/ge_p3_add.c \
    $$PWD/ed25519/additions/generalized/gen_eddsa.c \
    $$PWD/ed25519/additions/generalized/gen_labelset.c \
    $$PWD/ed25519/additions/generalized/gen_veddsa.c \
    $$PWD/ed25519/additions/generalized/gen_x.c \
    $$PWD/ed25519/additions/generalized/point_isreduced.c \
    $$PWD/ed25519/additions/generalized/sc_isreduced.c \
    $$PWD/ed25519/additions/xeddsa.c \
    $$PWD/ed25519/additions/zeroize.c \
    $$PWD/ed25519/nacl_sha512/blocks.c \
	$$PWD/ed25519/nacl_sha512/hash.c \
	$$PWD/ed25519/tests/internal_fast_tests.c

HEADERS += $$PWD/curve25519-donna.h \
	$$PWD/ed25519/additions/compare.h \
	$$PWD/ed25519/additions/crypto_additions.h \
	$$PWD/ed25519/additions/crypto_hash_sha512.h \
	$$PWD/ed25519/additions/curve_sigs.h \
	$$PWD/ed25519/additions/ed_sigs.h \
	$$PWD/ed25519/additions/keygen.h \
	$$PWD/ed25519/additions/utility.h \
	$$PWD/ed25519/additions/xeddsa.h \
	$$PWD/ed25519/additions/zeroize.h
