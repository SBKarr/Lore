
#include "SPStringView.h"

namespace stappler::languages {

enum class Locale {
	Unknown,
	_af,
	_af_NA,
	_af_ZA,
	_ak,
	_ak_GH,
	_am,
	_am_ET,
	_ar,
	_ar_AE,
	_ar_BH,
	_ar_DJ,
	_ar_DZ,
	_ar_EG,
	_ar_EH,
	_ar_ER,
	_ar_IL,
	_ar_IQ,
	_ar_JO,
	_ar_KM,
	_ar_KW,
	_ar_LB,
	_ar_LY,
	_ar_MA,
	_ar_MR,
	_ar_OM,
	_ar_PS,
	_ar_QA,
	_ar_SA,
	_ar_SD,
	_ar_SO,
	_ar_SS,
	_ar_SY,
	_ar_TD,
	_ar_TN,
	_ar_YE,
	_as,
	_as_IN,
	_az,
	_az_AZ,
	_az_Cyrl,
	_az_Cyrl_AZ,
	_az_Latn,
	_az_Latn_AZ,
	_be,
	_be_BY,
	_bg,
	_bg_BG,
	_bm,
	_bm_Latn,
	_bm_Latn_ML,
	_bn,
	_bn_BD,
	_bn_IN,
	_bo,
	_bo_CN,
	_bo_IN,
	_br,
	_br_FR,
	_bs,
	_bs_BA,
	_bs_Cyrl,
	_bs_Cyrl_BA,
	_bs_Latn,
	_bs_Latn_BA,
	_ca,
	_ca_AD,
	_ca_ES,
	_ca_FR,
	_ca_IT,
	_cs,
	_cs_CZ,
	_cy,
	_cy_GB,
	_da,
	_da_DK,
	_da_GL,
	_de,
	_de_AT,
	_de_BE,
	_de_CH,
	_de_DE,
	_de_LI,
	_de_LU,
	_dz,
	_dz_BT,
	_ee,
	_ee_GH,
	_ee_TG,
	_el,
	_el_CY,
	_el_GR,
	_en,
	_en_AG,
	_en_AI,
	_en_AS,
	_en_AU,
	_en_BB,
	_en_BE,
	_en_BM,
	_en_BS,
	_en_BW,
	_en_BZ,
	_en_CA,
	_en_CC,
	_en_CK,
	_en_CM,
	_en_CX,
	_en_DG,
	_en_DM,
	_en_ER,
	_en_FJ,
	_en_FK,
	_en_FM,
	_en_GB,
	_en_GD,
	_en_GG,
	_en_GH,
	_en_GI,
	_en_GM,
	_en_GU,
	_en_GY,
	_en_HK,
	_en_IE,
	_en_IM,
	_en_IN,
	_en_IO,
	_en_JE,
	_en_JM,
	_en_KE,
	_en_KI,
	_en_KN,
	_en_KY,
	_en_LC,
	_en_LR,
	_en_LS,
	_en_MG,
	_en_MH,
	_en_MO,
	_en_MP,
	_en_MS,
	_en_MT,
	_en_MU,
	_en_MW,
	_en_MY,
	_en_NA,
	_en_NF,
	_en_NG,
	_en_NR,
	_en_NU,
	_en_NZ,
	_en_PG,
	_en_PH,
	_en_PK,
	_en_PN,
	_en_PR,
	_en_PW,
	_en_RW,
	_en_SB,
	_en_SC,
	_en_SD,
	_en_SG,
	_en_SH,
	_en_SL,
	_en_SS,
	_en_SX,
	_en_SZ,
	_en_TC,
	_en_TK,
	_en_TO,
	_en_TT,
	_en_TV,
	_en_TZ,
	_en_UG,
	_en_UM,
	_en_US,
	_en_VC,
	_en_VG,
	_en_VI,
	_en_VU,
	_en_WS,
	_en_ZA,
	_en_ZM,
	_en_ZW,
	_eo,
	_es,
	_es_AR,
	_es_BO,
	_es_CL,
	_es_CO,
	_es_CR,
	_es_CU,
	_es_DO,
	_es_EA,
	_es_EC,
	_es_ES,
	_es_GQ,
	_es_GT,
	_es_HN,
	_es_IC,
	_es_MX,
	_es_NI,
	_es_PA,
	_es_PE,
	_es_PH,
	_es_PR,
	_es_PY,
	_es_SV,
	_es_US,
	_es_UY,
	_es_VE,
	_et,
	_et_EE,
	_eu,
	_eu_ES,
	_fa,
	_fa_AF,
	_fa_IR,
	_ff,
	_ff_CM,
	_ff_GN,
	_ff_MR,
	_ff_SN,
	_fi,
	_fi_FI,
	_fo,
	_fo_FO,
	_fr,
	_fr_BE,
	_fr_BF,
	_fr_BI,
	_fr_BJ,
	_fr_BL,
	_fr_CA,
	_fr_CD,
	_fr_CF,
	_fr_CG,
	_fr_CH,
	_fr_CI,
	_fr_CM,
	_fr_DJ,
	_fr_DZ,
	_fr_FR,
	_fr_GA,
	_fr_GF,
	_fr_GN,
	_fr_GP,
	_fr_GQ,
	_fr_HT,
	_fr_KM,
	_fr_LU,
	_fr_MA,
	_fr_MC,
	_fr_MF,
	_fr_MG,
	_fr_ML,
	_fr_MQ,
	_fr_MR,
	_fr_MU,
	_fr_NC,
	_fr_NE,
	_fr_PF,
	_fr_PM,
	_fr_RE,
	_fr_RW,
	_fr_SC,
	_fr_SN,
	_fr_SY,
	_fr_TD,
	_fr_TG,
	_fr_TN,
	_fr_VU,
	_fr_WF,
	_fr_YT,
	_fy,
	_fy_NL,
	_ga,
	_ga_IE,
	_gd,
	_gd_GB,
	_gl,
	_gl_ES,
	_gu,
	_gu_IN,
	_gv,
	_gv_IM,
	_ha,
	_ha_GH,
	_ha_Latn,
	_ha_Latn_GH,
	_ha_Latn_NE,
	_ha_Latn_NG,
	_ha_NE,
	_ha_NG,
	_he,
	_he_IL,
	_hi,
	_hi_IN,
	_hr,
	_hr_BA,
	_hr_HR,
	_hu,
	_hu_HU,
	_hy,
	_hy_AM,
	_id,
	_id_ID,
	_ig,
	_ig_NG,
	_ii,
	_ii_CN,
	_is,
	_is_IS,
	_it,
	_it_CH,
	_it_IT,
	_it_SM,
	_ja,
	_ja_JP,
	_ka,
	_ka_GE,
	_ki,
	_ki_KE,
	_kk,
	_kk_Cyrl,
	_kk_Cyrl_KZ,
	_kk_KZ,
	_kl,
	_kl_GL,
	_km,
	_km_KH,
	_kn,
	_kn_IN,
	_ko,
	_ko_KP,
	_ko_KR,
	_ks,
	_ks_Arab,
	_ks_Arab_IN,
	_ks_IN,
	_kw,
	_kw_GB,
	_ky,
	_ky_Cyrl,
	_ky_Cyrl_KG,
	_ky_KG,
	_lb,
	_lb_LU,
	_lg,
	_lg_UG,
	_ln,
	_ln_AO,
	_ln_CD,
	_ln_CF,
	_ln_CG,
	_lo,
	_lo_LA,
	_lt,
	_lt_LT,
	_lu,
	_lu_CD,
	_lv,
	_lv_LV,
	_mg,
	_mg_MG,
	_mk,
	_mk_MK,
	_ml,
	_ml_IN,
	_mn,
	_mn_Cyrl,
	_mn_Cyrl_MN,
	_mn_MN,
	_mr,
	_mr_IN,
	_ms,
	_ms_BN,
	_ms_Latn,
	_ms_Latn_BN,
	_ms_Latn_MY,
	_ms_Latn_SG,
	_ms_MY,
	_ms_SG,
	_mt,
	_mt_MT,
	_my,
	_my_MM,
	_nb,
	_nb_NO,
	_nb_SJ,
	_nd,
	_nd_ZW,
	_ne,
	_ne_IN,
	_ne_NP,
	_nl,
	_nl_AW,
	_nl_BE,
	_nl_BQ,
	_nl_CW,
	_nl_NL,
	_nl_SR,
	_nl_SX,
	_nn,
	_nn_NO,
	_no,
	_no_NO,
	_om,
	_om_ET,
	_om_KE,
	_or,
	_or_IN,
	_os,
	_os_GE,
	_os_RU,
	_pa,
	_pa_Arab,
	_pa_Arab_PK,
	_pa_Guru,
	_pa_Guru_IN,
	_pa_IN,
	_pa_PK,
	_pl,
	_pl_PL,
	_ps,
	_ps_AF,
	_pt,
	_pt_AO,
	_pt_BR,
	_pt_CV,
	_pt_GW,
	_pt_MO,
	_pt_MZ,
	_pt_PT,
	_pt_ST,
	_pt_TL,
	_qu,
	_qu_BO,
	_qu_EC,
	_qu_PE,
	_rm,
	_rm_CH,
	_rn,
	_rn_BI,
	_ro,
	_ro_MD,
	_ro_RO,
	_ru,
	_ru_BY,
	_ru_KG,
	_ru_KZ,
	_ru_MD,
	_ru_RU,
	_ru_UA,
	_rw,
	_rw_RW,
	_se,
	_se_FI,
	_se_NO,
	_se_SE,
	_sg,
	_sg_CF,
	_sh,
	_sh_BA,
	_si,
	_si_LK,
	_sk,
	_sk_SK,
	_sl,
	_sl_SI,
	_sn,
	_sn_ZW,
	_so,
	_so_DJ,
	_so_ET,
	_so_KE,
	_so_SO,
	_sq,
	_sq_AL,
	_sq_MK,
	_sq_XK,
	_sr,
	_sr_BA,
	_sr_Cyrl,
	_sr_Cyrl_BA,
	_sr_Cyrl_ME,
	_sr_Cyrl_RS,
	_sr_Cyrl_XK,
	_sr_Latn,
	_sr_Latn_BA,
	_sr_Latn_ME,
	_sr_Latn_RS,
	_sr_Latn_XK,
	_sr_ME,
	_sr_RS,
	_sr_XK,
	_sv,
	_sv_AX,
	_sv_FI,
	_sv_SE,
	_sw,
	_sw_CD,
	_sw_KE,
	_sw_TZ,
	_sw_UG,
	_ta,
	_ta_IN,
	_ta_LK,
	_ta_MY,
	_ta_SG,
	_te,
	_te_IN,
	_th,
	_th_TH,
	_ti,
	_ti_ER,
	_ti_ET,
	_tl,
	_tl_PH,
	_to,
	_to_TO,
	_tr,
	_tr_CY,
	_tr_TR,
	_ug,
	_ug_Arab,
	_ug_Arab_CN,
	_ug_CN,
	_uk,
	_uk_UA,
	_ur,
	_ur_IN,
	_ur_PK,
	_uz,
	_uz_AF,
	_uz_Arab,
	_uz_Arab_AF,
	_uz_Cyrl,
	_uz_Cyrl_UZ,
	_uz_Latn,
	_uz_Latn_UZ,
	_uz_UZ,
	_vi,
	_vi_VN,
	_yi,
	_yo,
	_yo_BJ,
	_yo_NG,
	_zh,
	_zh_CN,
	_zh_HK,
	_zh_Hans,
	_zh_Hans_CN,
	_zh_Hans_HK,
	_zh_Hans_MO,
	_zh_Hans_SG,
	_zh_Hant,
	_zh_Hant_HK,
	_zh_Hant_MO,
	_zh_Hant_TW,
	_zh_MO,
	_zh_SG,
	_zh_TW,
	_zu,
	_zu_ZA
};

enum class Language {
	Unknown,
	_ar /* Arabic */,
	_be /* Belarusian */,
	_bg /* Bulgarian */,
	_cs /* Czech */,
	_da /* Danish */,
	_de /* German */,
	_el /* Greek */,
	_en /* English */,
	_eo /* Esperanto */,
	_es /* Spanish */,
	_et /* Estonian */,
	_fa /* Persian */,
	_fi /* Finnish */,
	_fr /* French */,
	_ga /* Irish */,
	_hi /* Hindi */,
	_hu /* Hungarian */,
	_hy /* Armenian */,
	_is /* Icelandic */,
	_it /* Italian */,
	_ja /* Japanese */,
	_ka /* Georgian */,
	_kk /* Kazakh */,
	_ko /* Korean */,
	_ky /* Kyrgyz */,
	_la /* Latin */,
	_lt /* Lithuanian */,
	_lv /* Latvian */,
	_nl /* Dutch */,
	_pl /* Polish */,
	_pt /* Portuguese */,
	_ru /* Russian */,
	_sk /* Slovak */,
	_sl /* Slovenian */,
	_sq /* Albanian */,
	_sr /* Serbian */,
	_sv /* Swedish */,
	_ta /* Tamil */,
	_tg /* Tajik */,
	_th /* Thai */,
	_tr /* Turkish */,
	_uk /* Ukrainian */,
	_uz /* Uzbek */,
	_vi /* Vietnamese */,
	_zh /* Chinese */
};

Locale getLocale(StringView);
Language getLanguage(StringView);
StringView getLocaleCode(Locale);
StringView getLanguageCode(Language);
StringView getLanguageName(Language, Locale = Locale::Unknown);

}
