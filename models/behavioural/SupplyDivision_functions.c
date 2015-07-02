#include "header.h"
#include "SupplyDivision_agent_header.h"

int SD_report() {

  // Report output and cost statistics
  add_division_administration_report_message(FIRM_ID, DIVISION_ID,
    CUMULATIVE_OUTPUT / OUTPUT_TARGET, CUMULATIVE_COSTS, FUNDS);

  // Reset output and cost statistics
  FUNDS = 0;
  CUMULATIVE_OUTPUT = 0;
  CUMULATIVE_COSTS = 0;

  return 0;

}

int SD_process_instruction() {

  return 0;

}
