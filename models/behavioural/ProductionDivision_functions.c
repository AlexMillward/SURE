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

// TODO : expenditure safety checks and ? output adjustment (required?)
int PD_request_fixed_capital() {

  // Initialise
  double price = -1;

  // Get the market price
  START_MARKET_PRICE_MESSAGE_LOOP
    price = market_price_message->price;
  FINISH_MARKET_PRICE_MESSAGE_LOOP

  // Check that a message from the fixed capital supply market was actually found
  if (price != -1) {

    double requirement;

    requirement = ((double) OUTPUT_TARGET) * FIXED_CAPITAL_REQUIREMENT_PER_UNIT *
      (1 + FIXED_CAPITAL_REQUIREMENT_SLACK);

    // Check additional fixed capital is actually required
    if (requirement > FIXED_CAPITAL) {

      // Calculate quantity to order
      double to_order = requirement - FIXED_CAPITAL;

      // Send order
      add_market_order_message(FIXED_CAPITAL_MARKET_ID,
        FIRM_ID, DIVISION_ID, to_order);

    }

  }

  return 0;

}

int PD_add_fixed_capital_delivery() {

  START_MARKET_ORDER_CONFIRMATION_MESSAGE_LOOP

    // Adjust available funds
    FUNDS -= market_order_confirmation_message->price *
      market_order_confirmation_message->quantity;

    // Add the delivery
    add_single_type_delivery(&FIXED_CAPITAL_DELIVERIES,
      market_order_confirmation_message->quantity,
      FIXED_CAPITAL_INSTALLATION_TIME);

  FINISH_MARKET_ORDER_CONFIRMATION_MESSAGE_LOOP

  return 0;

}

/*

int PD_request_labour() {

  // Initialise
  double price = -1;

  // Get the market price
  START_MARKET_PRICE_MESSAGE_LOOP
    price = market_price_message->price;
  FINISH_MARKET_PRICE_MESSAGE_LOOP

  if (price != -1) {

    double requirement;

  }

  return 0;

}

*/
