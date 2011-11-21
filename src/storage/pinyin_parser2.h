/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PINYIN_PARSER2_H
#define PINYIN_PARSER2_H

#include <glib.h>
#include "chewing_key.h"
#include "pinyin_custom2.h"

namespace pinyin{

typedef struct {
    const char * m_pinyin_str;
    const char * m_chewing_str;
    ChewingKey   m_chewing_key;
} content_table_item_t;

typedef struct {
    const char * m_pinyin_input;
    guint32      m_flags;
    guint16      m_table_index;
} pinyin_index_item_t;

typedef struct {
    const char * m_chewing_input;
    guint32      m_flags;
    guint16      m_table_index;
} chewing_index_item_t;

typedef struct {
    ChewingKey   m_orig_key;
    guint32      m_orig_freq;
    ChewingKey   m_new_keys[2];
    guint32      m_new_freq;
} divided_table_item_t;

typedef struct {
    ChewingKey   m_orig_keys[2];
    guint32      m_orig_freq;
    ChewingKey   m_new_keys[2];
    guint32      m_new_freq;
} resplit_table_item_t;

typedef struct {
    const char * m_shengmu;
} double_pinyin_scheme_shengmu_item_t;

typedef struct {
    const char * m_yunmus[2];
} double_pinyin_scheme_yunmu_item_t;

typedef GArray * ChewingKeyVector;
typedef GArray * ChewingKeyRestVector;
typedef GArray * ParseValueVector;


/**
 * @brief Class to translate string into ChewingKey.
 */
class PinyinParser2
{
    /* constructor/destructor */
public:
    virtual ~PinyinParser2 () {}

    /* public method */
public:
    /**
     * @brief Translate only one ChewingKey from a string.
     *
     * @param options pinyin options from pinyin_custom2.h.
     * @param key stores result ChewingKey.
     * @param str snput string in UTF-8 encoding, in most case this string is just a plain ASCII string.
     * @param len the length of str, in number of chars rather than bytes.
     *
     * @return whether the entire string is parsed as one key.
     */
    virtual bool parse_one_key (guint32 options, ChewingKey & key, ChewingKeyRest & key_rest, const char *str, int len) const = 0;

    /**
     * @brief Translate the source string into a set of ChewingKeys.
     *
     * @param options pinyin options from pinyin_custom2.h.
     * @param keys stores result ChewingKeys.
     * @param str input string in UTF-8 encoding, in most case this string is just a plain ASCII string.
     * @param len the length of str, in number of chars rather than bytes.
     *
     * @return the number of chars were actually used.
     */
    /* Note:
     *   the parse method will use dynamic programming to drive parse_one_key.
     */
    virtual int parse (guint32 options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const = 0;

};


/**
 * The Full Pinyin Parser which parses full pinyin string into ChewingKeys.
 */
class FullPinyinParser2 : public PinyinParser2
{
    /* Note: some internal pointers to full pinyin table. */

protected:
    ParseValueVector m_parse_steps;

    int final_step(size_t step_len, ChewingKeyVector & keys,
                   ChewingKeyRestVector & key_rests) const;

    bool post_process(guint32 options, ChewingKeyVector & keys,
                      ChewingKeyRestVector & key_rests) const;

public:
    FullPinyinParser2 ();
    virtual ~FullPinyinParser2 () {
        g_array_free(m_parse_steps, TRUE);
    }

    virtual bool parse_one_key (guint32 options, ChewingKey & key, ChewingKeyRest & key_rest, const char *str, int len) const;

    virtual int parse (guint32 options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const;
};


/* The valid input chars of ShuangPin is a-z and ';'
 */
class DoublePinyinParser2 : public PinyinParser2
{
    /* Note: two internal pointers to double pinyin scheme table. */

public:
    virtual ~DoublePinyinParser2 () {}

    virtual bool parse_one_key (guint32 options, ChewingKey & key, ChewingKeyRest & key_rest, const char *str, int len) const;

    virtual int parse (guint32 options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const;

public:
    bool set_scheme (DoublePinyinScheme scheme);
};


/**
 * @brief Class to parse Chewing input string
 *
 * Several keyboard scheme are supported:
 * * Chewing_STANDARD  Standard ZhuYin keyboard, which maps 1 to Bo(ㄅ), q to Po(ㄆ) etc.
 * * Chewing_HSU       Hsu ZhuYin keyboard, which uses a-z (except q) chars.
 * * Chewing_IBM       IBM ZhuYin keyboard, which maps 1 to Bo(ㄅ), 2 to Po(ㄆ) etc.
 * * Chewing_GIN_YIEH  Gin-Yieh ZhuYin keyboard.
 * * Chewing_ET        Eten (倚天) ZhuYin keyboard.
 * * Chewing_ET26      Eten (倚天) ZhuYin keyboard, which only uses a-z chars.
 */
class ChewingParser2 : public PinyinParser2
{
    /* Note: one internal pointer to chewing scheme table. */

public:
    virtual ~ChewingParser2 () {}

    virtual bool parse_one_key (guint32 options, ChewingKey & key, ChewingKeyRest & key_rest, const char *str, int len) const;

    virtual int parse (guint32 options, ChewingKeyVector & keys, ChewingKeyRestVector & key_rests, const char *str, int len) const;

public:
    bool set_scheme (ChewingScheme scheme);
};


/* compare pinyins with chewing internal representations. */
inline int pinyin_compare_initial2 (guint32 options,
                                    ChewingInitial lhs,
                                    ChewingInitial rhs){
    if (lhs == rhs)
        return 0;

    if ((options & PINYIN_AMB_C_CH) &&
        ((lhs == CHEWING_C && rhs == CHEWING_CH) ||
         (lhs == CHEWING_CH && rhs == CHEWING_C)))
        return 0;

    if ((options & PINYIN_AMB_S_SH) &&
        ((lhs == CHEWING_S && rhs == CHEWING_SH) ||
         (lhs == CHEWING_SH && rhs == CHEWING_S)))
        return 0;

    if ((options & PINYIN_AMB_Z_ZH) &&
        ((lhs == CHEWING_Z && rhs == CHEWING_ZH) ||
         (lhs == CHEWING_ZH && rhs == CHEWING_Z)))
        return 0;

    if ((options & PINYIN_AMB_F_H) &&
        ((lhs == CHEWING_F && rhs == CHEWING_H) ||
         (lhs == CHEWING_H && rhs == CHEWING_F)))
        return 0;

    if ((options & PINYIN_AMB_L_N) &&
        ((lhs == CHEWING_L && rhs == CHEWING_N) ||
         (lhs == CHEWING_N && rhs == CHEWING_L)))
        return 0;

    if ((options & PINYIN_AMB_L_R) &&
        ((lhs == CHEWING_L && rhs == CHEWING_R) ||
         (lhs == CHEWING_R && rhs == CHEWING_L)))
        return 0;

    if ((options & PINYIN_AMB_G_K) &&
        ((lhs == CHEWING_G && rhs == CHEWING_K) ||
         (lhs == CHEWING_K && rhs == CHEWING_G)))
        return 0;

    return (lhs - rhs);
}


inline int pinyin_compare_middle_and_final2 (guint32 options,
                                             ChewingMiddle middle_lhs,
                                             ChewingMiddle middle_rhs,
                                             ChewingFinal final_lhs,
                                             ChewingFinal final_rhs){
    if (middle_lhs == middle_rhs && final_lhs == final_rhs)
        return 0;

    /* both pinyin and chewing incomplete options will enable this. */
    if (options & (PINYIN_INCOMPLETE | CHEWING_INCOMPLETE)) {
        if (middle_lhs == CHEWING_ZERO_MIDDLE &&
            final_lhs == CHEWING_ZERO_FINAL)
            return 0;
        if (middle_rhs == CHEWING_ZERO_MIDDLE &&
            final_rhs == CHEWING_ZERO_FINAL)
            return 0;
    }

    /* compare chewing middle first. */
    int middle_diff = middle_lhs - middle_rhs;
    if (middle_diff)
        return middle_diff;

    if ((options & PINYIN_AMB_AN_ANG) &&
        ((final_lhs == CHEWING_AN && final_rhs == CHEWING_ANG) ||
         (final_lhs == CHEWING_ANG && final_rhs == CHEWING_AN)))
        return 0;

    if ((options & PINYIN_AMB_EN_ENG) &&
        ((final_lhs == CHEWING_EN && final_rhs == CHEWING_ENG) ||
         (final_lhs == CHEWING_ENG && final_rhs == CHEWING_EN)))
        return 0;

    if ((options & PINYIN_AMB_IN_ING) &&
        ((final_lhs == PINYIN_IN && final_rhs == PINYIN_ING) ||
         (final_lhs == PINYIN_ING && final_rhs == PINYIN_IN)))
        return 0;

    return (final_lhs - final_rhs);
}


inline int pinyin_compare_tone2 (guint32 options,
                                 ChewingTone lhs,
                                 ChewingTone rhs){
    if (lhs == rhs)
        return 0;
    if (lhs == CHEWING_ZERO_TONE)
        return 0;
    if (rhs == CHEWING_ZERO_TONE)
        return 0;
    return (lhs - rhs);
}


};

#endif
