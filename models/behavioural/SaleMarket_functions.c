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

int SM_require() {

  // Send market requirements to firms
  for (int p=0; p<PARTICIPANTS.size; p++) {
    add_market_requirement_message(MARKET_ID, PARTICIPANTS.array[p].firm_id,
      PARTICIPANTS.array[p].quantity_demanded);
  }

  return 0;

}

int SM_confirm_sale() {

  int p;

  START_MARKET_PROVISION_MESSAGE_LOOP

    // Find relevant participant details
    for (p=0; p<PARTICIPANTS.size; p++) {
      if (PARTICIPANTS.array[p].firm_id == market_provision_message->firm_id) {

        // Confirm sale, stating deficit of quantity provided, quantity provided and price
        add_market_sale_confirmation_message(MARKET_ID,
          market_provision_message->firm_id,
          market_provision_message->quantity,
          PARTICIPANTS.array[p].quantity_demanded,
          PARTICIPANTS.array[p].price);

        break;
      }
    }

  FINISH_MARKET_PROVISION_MESSAGE_LOOP

  return 0;

}
