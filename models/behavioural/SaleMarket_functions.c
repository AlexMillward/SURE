#include <math.h>
#include "header.h"
#include "SaleMarket_agent_header.h"

int SM_update_demand() {

  // Initialisation
  int p, m;

  // Determine pre-marketing preference
  for (p=0; p<PARTICIPANTS.size; p++) {
    PARTICIPANTS.array[p].market_preference =
      (1 / PARTICIPANTS.array[p].price) * PARTICIPANTS.array[p].quality;
  }

  // Find totals of each marketing factor
  for (m=0; m<MARKETING_FACTOR_COUNT; m++) {
    MARKETING_FACTOR_TOTALS[m] = 0;

    // Add to marketing factor totals
    for (p=0; p<PARTICIPANTS.size; p++) {
      MARKETING_FACTOR_TOTALS[m] += PARTICIPANTS.array[p].marketing_factors[m];
    }
  }

  // Update preferences
  for (p=0; p<PARTICIPANTS.size; p++) {
    for (m=0; m<MARKETING_FACTOR_COUNT; m++) {
      PARTICIPANTS.array[p].market_preference *= (MARKETING_FACTOR_WEIGHTS[m] *
        (PARTICIPANTS.array[p].marketing_factors[m] / MARKETING_FACTOR_TOTALS[m]));
    }
  }

  // Calculate the preference total
  int new_preference_total = 0;
  for (p=0; p<PARTICIPANTS.size; p++) {
    new_preference_total += PARTICIPANTS.array[p].market_preference;
  }

  // Calculate change in overall demand and update the preference total
  OVERALL_DEMAND = floor(OVERALL_DEMAND * ((new_preference_total - PREFERENCE_TOTAL) / PREFERENCE_TOTAL));
  PREFERENCE_TOTAL = new_preference_total;

  // Calculate market share and quantity demanded for each participant
  for (p=0; p<PARTICIPANTS.size; p++) {
    PARTICIPANTS.array[p].market_share =
      PARTICIPANTS.array[p].market_preference / PREFERENCE_TOTAL;
    PARTICIPANTS.array[p].quantity_demanded =
      floor(PARTICIPANTS.array[p].market_share * OVERALL_DEMAND);
  }

  return 0;

}
