/*
 * PokerUnicorn Server
 * This project uses test network, NO real coin or NO real money involved.
 * Copyright (C) 2023, Oğuzhan Eroğlu <meowingcate@gmail.com> (https://meowingcat.io)
 * Licensed under GPLv3 License
 * See LICENSE for more info
 */

#include <stdbool.h>
#include <stdio.h>

#include "../include/card.h"
#include "../include/random.h"
#include "../include/poker.h"

pkrsrv_card_t pkrsrv_card_get_random(pkrsrv_card_t* history) {
    pkrsrv_card_t generated;
    
    GENERATE:
    
    generated = (pkrsrv_card_t) {
        .index = pkrsrv_random_range(0, 13),
        .kind = pkrsrv_random_range(0, 3)
    };

    int current_i = 0;

    for (int i=0; i < 52; i++) {
        pkrsrv_card_t previous = history[i];
        
        current_i = i;

        if (previous.index < 0) {
            break;
        }

        if ((previous.index == generated.index) && (previous.kind == generated.kind)) {
            goto GENERATE;
        }
    }

    history[current_i] = generated;

    return generated;
}

void pkrsrv_card_sort_sevens(pkrsrv_card_t* sevens) {
    for (int i=0; i < 7; i++) {
        for (int j=0; j < 7; j++) {
            if (sevens[i].index > sevens[j].index) {
                pkrsrv_card_t tmp = sevens[i];
                sevens[i] = sevens[j];
                sevens[j] = tmp;
            }
        }
    }
}

void pkrsrv_card_get_fives(pkrsrv_card_t* sevens, pkrsrv_card_t* target) {
    for (int i=0; i < 21; i++) {
        int* indexes = FIVES_INDEXES + (i * 5);
        
        for (int j=0; j < 5; j++) {
            target[i * 5 + j].index = sevens[indexes[j]].index;
            target[i * 5 + j].kind = sevens[indexes[j]].kind;
        }
    }
}

pkrsrv_card_score_result_t pkrsrv_card_get_get_fives_score(pkrsrv_card_t* fives) {
    pkrsrv_card_score_result_t result = {
        .is_high_card = false
    };

    pkrsrv_card_check_result_t royal_flush = pkrsrv_card_check_royal_flush(fives);
    if ((result.is_royal_flush = royal_flush.is_exists)) {
        result.check_result = royal_flush;
        goto RETURN;
    }
    
    pkrsrv_card_check_result_t straight_flush = pkrsrv_card_check_straight_flush(fives);
    if ((result.is_straight_flush = straight_flush.is_exists)) {
        result.check_result = straight_flush;
        goto RETURN;
    }

    pkrsrv_card_check_result_t quad = pkrsrv_card_check_quad(fives);
    if ((result.is_quad = quad.is_exists)) {
        result.check_result = quad;
        goto RETURN;
    }
    
    pkrsrv_card_check_result_t full_house = pkrsrv_card_check_full_house(fives);
    if ((result.is_full_house = full_house.is_exists)) {
        result.check_result = full_house;
        goto RETURN;
    }

    pkrsrv_card_check_result_t flush = pkrsrv_card_check_flush(fives);
    if ((result.is_flush = flush.is_exists)) {
        result.check_result = flush;
        goto RETURN;
    }

    pkrsrv_card_check_result_t straight = pkrsrv_card_check_straight(fives);
    if ((result.is_straight = straight.is_exists)) {
        result.check_result = straight;
        goto RETURN;
    }

    pkrsrv_card_check_result_t three_kind = pkrsrv_card_check_three_kind(fives);
    if ((result.is_three_kind = three_kind.is_exists)) {
        result.check_result = three_kind;
        goto RETURN;
    }

    pkrsrv_card_check_result_t two_pair = pkrsrv_card_check_two_pair(fives);
    if (result.is_two_pair = two_pair.is_exists) {
        result.check_result = two_pair;
        goto RETURN;
    }
    
    pkrsrv_card_check_result_t pair = pkrsrv_card_check_pair(fives);
    if ((result.is_pair = pair.is_exists)) {
        result.check_result = pair;
        goto RETURN;
    }

    pkrsrv_card_check_result_t high_card = pkrsrv_card_check_high_card(fives);
    result.is_high_card = true;
    result.check_result = high_card;

    RETURN:
    
    if (result.is_royal_flush) {
    } else if (result.is_straight_flush) {
        result.check_result.rank += (result.check_result.cards[4].index + 1) * 10000;
    } else if (result.is_quad) {
        result.check_result.rank += (result.check_result.cards[0].index + 1) * 10000;
        result.check_result.rank += (result.check_result.cards[4].index + 1) * 100;
    } else if (result.is_full_house) {
        result.check_result.rank += (result.check_result.cards[0].index + 1) * 10000;
        result.check_result.rank += (result.check_result.cards[4].index + 1);
    } else if (result.is_flush) {
        result.check_result.rank += (result.check_result.cards[0].index + 1) * 10000;
    } else if (result.is_straight) {
        result.check_result.rank += (result.check_result.cards[4].index + 1) * 10000;
    } else if (result.is_three_kind) {
        result.check_result.rank += (result.check_result.cards[0].index + 1) * 10000;
        result.check_result.rank += (result.check_result.cards[4].index + 1);
    } else if (result.is_two_pair) {
        result.check_result.rank += (result.check_result.cards[2].index + 1) * 10000;
        result.check_result.rank += (result.check_result.cards[0].index + 1) * 100;
        result.check_result.rank += (result.check_result.cards[4].index + 1);
    } else if (result.is_pair) {
        result.check_result.rank += (result.check_result.cards[0].index + 1) * 10000;
        result.check_result.rank += (result.check_result.cards[4].index + 1);
    } else {
        result.check_result.rank += (result.check_result.cards[4].index + 1) * 10000;
    }


    return result;
}

pkrsrv_card_score_result_t pkrsrv_card_get_best_fives_score(pkrsrv_card_t* sevens) {
    pkrsrv_card_sort_sevens(sevens);

    pkrsrv_card_t all_fives[21 * 5];
    pkrsrv_card_get_fives(sevens, all_fives);

    pkrsrv_card_score_result_t highest = {0};

    for (int i=0; i < 21; i++) {
        pkrsrv_card_t* fives = (all_fives + i * 5);
        pkrsrv_card_score_result_t score = pkrsrv_card_get_get_fives_score(fives);

        if (score.check_result.rank > highest.check_result.rank) {
            highest = score;
        }
    }

    return highest;
}

pkrsrv_card_check_result_t pkrsrv_card_check_high_card(pkrsrv_card_t* fives) {
    pkrsrv_card_check_result_t result = {
        .is_exists = false,
        .rank = PKRSRV_CARD_HAND_RANK_HIGH_CARD
    };
    
    result.cards[0] = fives[0];
    result.cards[1] = fives[1];
    result.cards[2] = fives[2];
    result.cards[3] = fives[3];
    result.cards[4] = fives[4];

    return result;
}

pkrsrv_card_check_result_t pkrsrv_card_check_two_pair(pkrsrv_card_t* fives) {
    pkrsrv_card_check_result_t result = {
        .is_exists = false,
        .rank = PKRSRV_CARD_HAND_RANK_TWO_PAIR
    };

    if ((fives[0].index == fives[1].index) && (fives[2].index == fives[3].index)) {
        result.rank = PKRSRV_CARD_HAND_RANK_TWO_PAIR;
        result.is_exists = true;
        result.cards[0] = fives[0];
        result.cards[1] = fives[1];
        result.cards[2] = fives[2];
        result.cards[3] = fives[3];
        result.cards[4] = fives[4];
    } else if ((fives[0].index == fives[1].index) && (fives[3].index == fives[4].index)) {
        result.is_exists = true;
        result.cards[0] = fives[0];
        result.cards[1] = fives[1];
        result.cards[2] = fives[3];
        result.cards[3] = fives[4];
        result.cards[4] = fives[2];
    } else if ((fives[1].index == fives[2].index) && (fives[3].index == fives[4].index)) {
        result.is_exists = true;
        result.cards[0] = fives[1];
        result.cards[1] = fives[2];
        result.cards[2] = fives[3];
        result.cards[3] = fives[4];
        result.cards[4] = fives[0];
    }

    return result;
}
pkrsrv_card_check_result_t pkrsrv_card_check_pair(pkrsrv_card_t* fives) {
    pkrsrv_card_check_result_t result = {
        .is_exists = false,
        .rank = PKRSRV_CARD_HAND_RANK_PAIR
    };

    if ((fives[0].index == fives[1].index)) {
        result.is_exists = true;
        result.cards[0] = fives[0];
        result.cards[1] = fives[1];
        result.cards[2] = fives[2];
        result.cards[3] = fives[3];
        result.cards[4] = fives[4];
    } else if ((fives[1].index == fives[2].index)) {
        result.is_exists = true;
        result.cards[0] = fives[1];
        result.cards[1] = fives[2];
        result.cards[2] = fives[0];
        result.cards[3] = fives[3];
        result.cards[4] = fives[4];
    } else if ((fives[2].index == fives[3].index)) {
        result.is_exists = true;
        result.cards[0] = fives[2];
        result.cards[1] = fives[3];
        result.cards[2] = fives[0];
        result.cards[3] = fives[1];
        result.cards[4] = fives[4];
    } else if ((fives[3].index == fives[4].index)) {
        result.is_exists = true;
        result.cards[0] = fives[3];
        result.cards[1] = fives[4];
        result.cards[2] = fives[0];
        result.cards[3] = fives[1];
        result.cards[4] = fives[2];
    }

    return result;
}

pkrsrv_card_check_result_t pkrsrv_card_check_three_kind(pkrsrv_card_t* fives) {
    pkrsrv_card_check_result_t result = {
        .is_exists = false,
        .rank = PKRSRV_CARD_HAND_RANK_THREE_KIND
    };

    if ((fives[0].index == fives[1].index) && (fives[1].index == fives[2].index)) {
        result.is_exists = true;
        result.cards[0] = fives[0];
        result.cards[1] = fives[1];
        result.cards[2] = fives[2];
        result.cards[3] = fives[3];
        result.cards[4] = fives[4];
    } else if ((fives[1].index == fives[2].index) && (fives[2].index == fives[3].index)) {
        result.is_exists = true;
        result.cards[0] = fives[1];
        result.cards[1] = fives[2];
        result.cards[2] = fives[3];
        result.cards[3] = fives[0];
        result.cards[4] = fives[4];
    } else if ((fives[2].index == fives[3].index) && (fives[3].index == fives[4].index)) {
        result.is_exists = true;
        result.cards[0] = fives[2];
        result.cards[1] = fives[3];
        result.cards[2] = fives[4];
        result.cards[3] = fives[0];
        result.cards[4] = fives[1];
    }

    return result;
}

pkrsrv_card_check_result_t pkrsrv_card_check_straight(pkrsrv_card_t* fives) {
    pkrsrv_card_check_result_t result = {
        .is_exists = false,
        .rank = PKRSRV_CARD_HAND_RANK_STRAIGHT
    };

    result.is_exists = (fives[0].index == (fives[1].index - 1)) &&
                       (fives[1].index == (fives[2].index - 1)) &&
                       (fives[2].index == (fives[3].index - 1)) &&
                       (fives[3].index == (fives[4].index - 1));

    if (result.is_exists) {
        result.cards[0] = fives[0];
        result.cards[1] = fives[1];
        result.cards[2] = fives[2];
        result.cards[3] = fives[3];
        result.cards[4] = fives[4];
    }
    
    return result;
}

pkrsrv_card_check_result_t pkrsrv_card_check_flush(pkrsrv_card_t* fives) {
    pkrsrv_card_check_result_t result = {
        .is_exists = false,
        .rank = PKRSRV_CARD_HAND_RANK_FLUSH
    };

    result.is_exists = (fives[0].kind == fives[1].kind) &&
                       (fives[1].kind == fives[2].kind) &&
                       (fives[2].kind == fives[3].kind) &&
                       (fives[3].kind == fives[4].kind);

    if (result.is_exists) {
        result.cards[0] =  fives[0];
        result.cards[1] =  fives[1];
        result.cards[2] =  fives[2];
        result.cards[3] =  fives[3];
        result.cards[4] =  fives[4];
    }

    return result;
}

pkrsrv_card_check_result_t pkrsrv_card_check_full_house(pkrsrv_card_t* fives) {
    pkrsrv_card_check_result_t result = {
        .is_exists = false,
        .rank = PKRSRV_CARD_HAND_RANK_FULL_HOUSE
    };

    if ((fives[0].index == fives[1].index) && (fives[1].index == fives[2].index) && (fives[3].index == fives[4].index)) {
        result.is_exists = true;
        result.cards[0] = fives[0];
        result.cards[1] = fives[1];
        result.cards[2] = fives[2];
        result.cards[3] = fives[3];
        result.cards[4] = fives[4];
    } else if ((fives[0].index == fives[1].index) && (fives[2].index == fives[3].index) && (fives[3].index == fives[4].index)) {
        result.is_exists = true;
        result.cards[0] = fives[2];
        result.cards[1] = fives[3];
        result.cards[2] = fives[4];
        result.cards[3] = fives[0];
        result.cards[4] = fives[1];
    }

    return result;
}

pkrsrv_card_check_result_t pkrsrv_card_check_quad(pkrsrv_card_t* fives) {
    pkrsrv_card_check_result_t result = {
        .is_exists = false,
        .rank = PKRSRV_CARD_HAND_RANK_QUAD
    };

    if ((fives[0].index == fives[1].index) && (fives[1].index == fives[2].index) && (fives[2].index == fives[3].index)) {
        result.is_exists = true;
        result.cards[0] = fives[0];
        result.cards[1] = fives[1];
        result.cards[2] = fives[2];
        result.cards[3] = fives[3];
        result.cards[4] = fives[4];
    } else if ((fives[1].index == fives[2].index) && (fives[2].index == fives[3].index) && (fives[3].index == fives[4].index)) {
        result.is_exists = true;
        result.cards[0] = fives[1];
        result.cards[1] = fives[2];
        result.cards[2] = fives[3];
        result.cards[3] = fives[4];
        result.cards[4] = fives[0];
    }

    return result;
}

pkrsrv_card_check_result_t pkrsrv_card_check_straight_flush(pkrsrv_card_t* fives) {
    pkrsrv_card_check_result_t result = {
        .is_exists = false,
        .rank = PKRSRV_CARD_HAND_RANK_STRAIGHT_FLUSH
    };

    bool is_straight = (fives[0].index == (fives[1].index - 1)) &&
                       (fives[1].index == (fives[2].index - 1)) &&
                       (fives[2].index == (fives[3].index - 1)) &&
                       (fives[3].index == (fives[4].index - 1));

    if (!is_straight) {
        return result;
    }

    if (
        (fives[0].kind == fives[1].kind) &&
        (fives[1].kind == fives[2].kind) &&
        (fives[2].kind == fives[3].kind) &&
        (fives[3].kind == fives[4].kind)
    ) {
        result.is_exists = true;
        result.cards[0] = fives[0];
        result.cards[1] = fives[1];
        result.cards[2] = fives[2];
        result.cards[3] = fives[3];
        result.cards[4] = fives[4];
    }
    
    return result;
}

pkrsrv_card_check_result_t pkrsrv_card_check_royal_flush(pkrsrv_card_t* fives) {
    pkrsrv_card_check_result_t result = {
        .is_exists = false,
        .rank = PKRSRV_CARD_HAND_RANK_ROYAL_FLUSH
    };

    bool is_straight = (fives[0].index == (fives[1].index - 1)) &&
                       (fives[1].index == (fives[2].index - 1)) &&
                       (fives[2].index == (fives[3].index - 1)) &&
                       (fives[3].index == (fives[4].index - 1));

    if (!is_straight) {
        return result;
    }

    bool is_same_kind = (fives[0].kind == fives[1].kind) &&
                        (fives[1].kind == fives[2].kind) &&
                        (fives[2].kind == fives[3].kind) &&
                        (fives[3].kind == fives[4].kind);
    
    if (is_same_kind) {
        return result;
    }

    bool is_royal = (fives[0].index == 12) &&
                    (fives[1].index == 11) &&
                    (fives[2].index == 10) &&
                    (fives[3].index == 9) &&
                    (fives[4].index == 8);
    
    if (!is_royal) {
        return result;
    }

    return result;
}