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

    

  FINISH_DIVISION_MARKETING_INVENTORY_REPORT_MESSAGE_LOOP

  return 0;

}
