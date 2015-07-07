#include <math.h>
#include "header.h"
#include "SupplyDivision_agent_header.h"

int SD_report() {

  // Report output and cost statistics
  add_division_administration_report_message(FIRM_ID, DIVISION_ID,
    ((double) CUMULATIVE_OUTPUT) / ((double) OUTPUT_TARGET), CUMULATIVE_COSTS, FUNDS);

  // Reset output and cost statistics
  FUNDS = 0;
  CUMULATIVE_OUTPUT = 0;
  CUMULATIVE_COSTS = 0;

  return 0;

}

int SD_process_instruction() {

  START_DIVISION_PRODUCTION_INSTRUCTION_MESSAGE_LOOP

    // Set output target and funds from instruction
    OUTPUT_TARGET = division_production_instruction_message->production_target;
    FUNDS = division_production_instruction_message->funding;

  FINISH_DIVISION_PRODUCTION_INSTRUCTION_MESSAGE_LOOP

  return 0;

}

int SD_order() {

  // Retrieve the price
  double price = -1;
  START_MARKET_PRICE_MESSAGE_LOOP
    price = market_price_message->price;
  FINISH_MARKET_PRICE_MESSAGE_LOOP

  // Check a price was found
  if (price != -1) {

    // Purchase target
    int purchase_target = ceil(OUTPUT_TARGET / 20);

    // Potential purchase given funds
    int purchase_potential = floor(FUNDS / price);

    // Determine actual purchase amount
    int purchase_amount = purchase_target < purchase_potential ? purchase_target : purchase_potential;

    // Place order
    add_market_order_message(SOURCE_MARKET_ID, FIRM_ID, DIVISION_ID, purchase_amount);

  }

  return 0;

}

int SD_process_order_confirmation() {

  START_MARKET_ORDER_CONFIRMATION_MESSAGE_LOOP

    // Adjust funds and output tracking
    double costs = market_order_confirmation_message->price *
      market_order_confirmation_message->quantity;
    FUNDS -= costs;
    CUMULATIVE_COSTS += costs;
    CUMULATIVE_OUTPUT += market_order_confirmation_message->quantity;

    // Deliver to appropriate production division
    add_division_delivery_notification_message(FIRM_ID, DIVISION_ID, PARENT_DIVISION_ID,
      market_order_confirmation_message->quantity, DELIVERY_TIME);

  FINISH_MARKET_ORDER_CONFIRMATION_MESSAGE_LOOP

  return 0;

}
