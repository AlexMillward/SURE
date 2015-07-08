#include <math.h>
#include "header.h"
#include "SalesAndMarketingDivision_agent_header.h"

int SMD_report() {

  // Report costs
  add_division_administration_report_message(FIRM_ID, DIVISION_ID,
    -1, CUMULATIVE_COSTS, FUNDS);

  // Reset costs
  FUNDS = 0;
  CUMULATIVE_COSTS = 0;

  return 0;

}

int SMD_process_instruction() {

  START_DIVISION_MARKETING_INSTRUCTION_MESSAGE_LOOP

    // Set funds from instruction
    FUNDS = division_marketing_instruction_message->funding;

  FINISH_DIVISION_MARKETING_INSTRUCTION_MESSAGE_LOOP

  return 0;

}

int SMD_strategise() {

  START_DIVISION_MARKETING_INVENTORY_REPORT_MESSAGE_LOOP

    // Find change in sales, prepare previous sales for next iteration
    CHANGE_IN_SALES = division_marketing_inventory_report_message->dispatched -
      PREVIOUS_SALES;
    PROPORTION_SALES_CHANGE = ((double) CHANGE_IN_SALES) / ((double) PREVIOUS_SALES);
    PREVIOUS_SALES = division_marketing_inventory_report_message->dispatched;

  FINISH_DIVISION_MARKETING_INVENTORY_REPORT_MESSAGE_LOOP

  // Initialisation
  int s, i, history_index;
  double scores[MARKETING_FACTOR_COUNT];
  double cost_sum = 0;

  // Find sum of costs
  for (s=0; s<MARKETING_FACTOR_COUNT; s++) {
    cost_sum += FACTORS[s].cost;
  }

  // Calculate scores
  for (s=0; s<MARKETING_FACTOR_COUNT; s++) {
    scores[s] = FACTORS[s].cost / cost_sum;
    for (i=1; i<(HISTORY.size); i++) {
      history_index = HISTORY.size - i;
      scores[s] += ( 1.0 / pow(FACTOR_ALPHA, i) ) *
        (HISTORY.array[history_index].proportion_sales_change /
        (FACTOR_BETA * HISTORY.array[history_index - 1].factor_proportions[s]) );
    }
  }

  // Normalise scores
  double score_sum = 0;
  for (s=0; s<MARKETING_FACTOR_COUNT; s++) {
    score_sum += scores[s];
  }
  for (s=0; s<MARKETING_FACTOR_COUNT; s++) {
    scores[s] /= score_sum;
  }

  // Purchase factors
  for (s=0; s<MARKETING_FACTOR_COUNT; s++) {
    FACTORS[s].available += (scores[s] * FUNDS) / FACTORS[s].cost;
    FUNDS -= FACTORS[s].available * FACTORS[s].cost;
  }

  // Calculate factor total
  double factor_total = 0;
  for (s=0; s<MARKETING_FACTOR_COUNT; s++) {
    factor_total += FACTORS[s].available;
  }

  // Update history
  double history_additions[MARKETING_FACTOR_COUNT];
  for (s=0; s<MARKETING_FACTOR_COUNT; s++) {
    history_additions[s] = FACTORS[s].available;
  }
  add_division_sales_history(&HISTORY, PROPORTION_SALES_CHANGE, (&history_additions)[MARKETING_FACTOR_COUNT]);
  if (HISTORY.size > HISTORY_SIZE_LIMIT) {
    remove_division_sales_history(&HISTORY, 0);
  }

  return 0;

}

int SMD_apply_factors() {

  // Send description of marketing activites to the market
  double daily_factors[MARKETING_FACTOR_COUNT];
  for (int m=0; m<MARKETING_FACTOR_COUNT; m++) {
    daily_factors[m] = FACTORS[m].available / 20;
  }

  add_marketing_factors_message(FIRM_ID, daily_factors);

  return 0;

}
