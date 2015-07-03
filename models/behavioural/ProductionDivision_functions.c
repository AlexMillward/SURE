#include "header.h"
#include "ProductionDivision_agent_header.h"

int PD_report() {

  // Report output and cost statistics
  add_division_administration_report_message(FIRM_ID, DIVISION_ID,
    ((double) CUMULATIVE_OUTPUT) / ((double) OUTPUT_TARGET), CUMULATIVE_COSTS, FUNDS);

  // Reset output and cost statistics
  FUNDS = 0;
  CUMULATIVE_OUTPUT = 0;
  CUMULATIVE_COSTS = 0;

  return 0;

}

int PD_process_instruction() {

  START_DIVISION_PRODUCTION_INSTRUCTION_MESSAGE_LOOP

    // Set output target and funds from instruction
    OUTPUT_TARGET = division_production_instruction_message->production_target;
    FUNDS = division_production_instruction_message->funding;

  FINISH_DIVISION_PRODUCTION_INSTRUCTION_MESSAGE_LOOP

  return 0;

}
