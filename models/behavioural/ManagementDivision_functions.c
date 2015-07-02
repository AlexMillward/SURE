#include <math.h>
#include "header.h"
#include "ManagementDivision_agent_header.h"

int MD_administrate() {

  CUMULATIVE_COSTS = 0;

  // Reset production/supply division reports
  reset_division_production_report_array(&PRODUCTION_REPORTS);

  START_DIVISION_ADMINISTRATION_REPORT_MESSAGE_LOOP

    // Count up cumulative costs
    CUMULATIVE_COSTS += division_administration_report_message->cumulative_costs;
    FUNDS += division_administration_report_message->remaining_funds;

    // Check for production division
    if (division_administration_report_message->attainment > -0.5) {

      // Record costs and suggested funding to achieve output target
      add_division_production_report(&PRODUCTION_REPORTS,
        division_administration_report_message->division_id,
        division_administration_report_message->cumulative_costs,
        (1 + FUNDING_SLACK) * (1 / division_administration_report_message->attainment) *
          division_administration_report_message->cumulative_costs);

    }

  FINISH_DIVISION_ADMINISTRATION_REPORT_MESSAGE_LOOP

  // Reset inventory statistics
  CUMULATIVE_PRODUCED = 0;
  CUMULATIVE_SOLD = 0;
  CUMULATIVE_DEFICIT = 0;
  CURRENT_STOCK = 0;

  START_DIVISION_INVENTORY_REPORT_MESSAGE_LOOP

    // Count up inventory statistics
    CUMULATIVE_PRODUCED += division_inventory_report_message->recieved;
    CUMULATIVE_SOLD += division_inventory_report_message->dispatched;
    CUMULATIVE_DEFICIT += division_inventory_report_message->deficit;
    CURRENT_STOCK += division_inventory_report_message->remaining;

  FINISH_DIVISION_INVENTORY_REPORT_MESSAGE_LOOP

  // Decide pricing
  PRICE = (CUMULATIVE_COSTS / CUMULATIVE_PRODUCED) * MARKUP;

  // Allocation
  int p;

  // TODO : allocation of running costs (costs other than production/sales allocations)

  // Calculate the funding required to meet the current production target
  double funding_to_meet_target = 0;
  for (p=0; p<PRODUCTION_REPORTS.size; p++) {
    funding_to_meet_target += PRODUCTION_REPORTS.array[p].suggested_funding;
  }

  /*
    Estimate the proportion of the current production target demanded, and the
    proportion achievable with available funds
  */
  double proportion_demanded = ((CUMULATIVE_DEFICIT + CUMULATIVE_SOLD) / CUMULATIVE_SOLD);
  double proportion_achievable = (FUNDS / funding_to_meet_target);
  double adjust_by = proportion_demanded < proportion_achievable ? proportion_demanded : proportion_achievable;

  // Adjust the production target and initialise production expenditure for the iteration
  PRODUCTION_TARGET *= adjust_by;
  PRODUCTION_EXPENDITURE = 0;

  // Calculate division funding, send instruction to production, and supply divisions and adjust funds
  double division_funding;
  for (p=0; p<PRODUCTION_REPORTS.size; p++) {
    division_funding = PRODUCTION_REPORTS.array[p].suggested_funding * adjust_by;
    FUNDS -= division_funding;
    add_division_production_instruction_message(FIRM_ID, PRODUCTION_REPORTS.array[p].division_id,
      PRODUCTION_TARGET, division_funding);
  }

  // Calculate funds available for marketing activities
  double marketing_funding = FUNDS - PRODUCTION_FUNDING_BUFFER;

  // Tell marketing division how much funding is available
  add_division_marketing_instruction_message(FIRM_ID, marketing_funding);

  return 0;

}
