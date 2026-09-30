// shuffle_core.cpp
#include "shuffle_core.h"

bool slots_assign(Drbg* d, uint8_t n_cards, uint8_t n_slots, uint8_t slot_of_card[MAX_CARDS]) {
    if (n_cards > n_slots || n_slots > MAX_CARDS) return false;
    uint8_t empty[MAX_CARDS]; uint8_t len = n_slots;
    for (uint8_t s = 0; s < n_slots; s++) empty[s] = s;
    for (uint8_t i = 0; i < n_cards; i++) {
        uint8_t idx = (uint8_t)drbg_uniform(d, len);
        slot_of_card[i] = empty[idx];
        empty[idx] = empty[len - 1];
        len--;
    }
    return true;
}

void predict_order_from_slots(uint8_t n_cards, const uint8_t slot_of_card[MAX_CARDS], uint8_t order[MAX_CARDS]) {
    // position of card i = number of cards in slots lower than its own
    uint8_t p = 0;
    for (uint8_t s = 0; s < MAX_CARDS; s++)
        for (uint8_t i = 0; i < n_cards; i++)
            if (slot_of_card[i] == s) order[p++] = i;
}
