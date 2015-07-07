#include <math.h>
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

    // Get the daily requirement
    double requirement;
    requirement = ((((double) OUTPUT_TARGET) / 20) / DAILY_UNIT_FIXED_CAPITAL_OUTPUT) *
      (1 + FIXED_CAPITAL_REQUIREMENT_SLACK);

    // Check additional fixed capital is actually required
    if (requirement > FIXED_CAPITAL) {

      // Calculate quantity to order
      int to_order = ceil(requirement - FIXED_CAPITAL);

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
    double expenditure = market_order_confirmation_message->price *
      market_order_confirmation_message->quantity;
    FUNDS -= expenditure;
    CUMULATIVE_COSTS += expenditure;

    // Add the delivery
    add_single_type_record(&FIXED_CAPITAL_DELIVERIES,
      market_order_confirmation_message->quantity,
      FIXED_CAPITAL_INSTALLATION_TIME);

  FINISH_MARKET_ORDER_CONFIRMATION_MESSAGE_LOOP

  return 0;

}

int PD_request_labour() {

  // Initialise
  double price = -1;

  // Get the market price
  START_MARKET_PRICE_MESSAGE_LOOP
    price = market_price_message->price;
  FINISH_MARKET_PRICE_MESSAGE_LOOP

  if (price != -1) {

    int d;

    // Find the eventual quantity of fixed capital
    double eventual_fixed_capital = FIXED_CAPITAL;
    for (d=0; d<FIXED_CAPITAL_DELIVERIES.size; d++) {
      eventual_fixed_capital += FIXED_CAPITAL_DELIVERIES.array[d].quantity;
    }

    // Find the potential labour that could be used given current capital
    int fixed_capital_potential = floor(LABOUR_PER_UNIT_FIXED_CAPITAL * eventual_fixed_capital);

    // Find desirable based on output target (TODO : confirm equation)
    int desirable = ceil((((double) OUTPUT_TARGET / 20) / DAILY_UNIT_FIXED_CAPITAL_OUTPUT) *
      LABOUR_PER_UNIT_FIXED_CAPITAL * (1 + LABOUR_REQUIREMENT_SLACK));

    // Decide on the quantity to employ
    int employment_quantity = (desirable < fixed_capital_potential ? desirable : fixed_capital_potential);

    // Add the order
    add_market_order_message(LABOUR_MARKET_ID, FIRM_ID, DIVISION_ID, employment_quantity);

  }

  return 0;

}

int PD_record_labour() {

  START_MARKET_ORDER_CONFIRMATION_MESSAGE_LOOP

    // Adjust available funds
    double expenditure = market_order_confirmation_message->price *
      market_order_confirmation_message->quantity;
    FUNDS -= expenditure;
    CUMULATIVE_COSTS += expenditure;

    // Record the labour addition
    add_single_type_record(&UPCOMING_LABOUR_CONTRACTS,
      market_order_confirmation_message->quantity,
      LABOUR_EMPLOYMENT_PHASE);


  FINISH_MARKET_ORDER_CONFIRMATION_MESSAGE_LOOP

  return 0;

}

int PD_update_fixed_capital() {

  // Update deliveries
  int r=0;
  for (r=0; r<FIXED_CAPITAL_DELIVERIES.size; r++) {

    // Reduce counter to arrival
    FIXED_CAPITAL_DELIVERIES.array[r].time--;

    // Delivery arrived
    if (FIXED_CAPITAL_DELIVERIES.array[r].time <= 0) {

      // Adjust fixed capital
      FIXED_CAPITAL += FIXED_CAPITAL_DELIVERIES.array[r].quantity;

      // Remove record of delivery
      remove_single_type_record(&FIXED_CAPITAL_DELIVERIES, r);

    } else {
      r++;
    }

  }

  // Depreciate
  FIXED_CAPITAL *= (1 - FIXED_CAPITAL_DEPRECIATION_RATE);

  return 0;

}

int PD_update_labour() {

  // Update upcoming contracts
  int r;

  r=0;
  while (r<UPCOMING_LABOUR_CONTRACTS.size) {

    // Reduce counter to contract start
    UPCOMING_LABOUR_CONTRACTS.array[r].time--;

    // Contract starting
    if (UPCOMING_LABOUR_CONTRACTS.array[r].time <= 0) {

      // Update records
      add_single_type_record(&EXISTING_LABOUR_CONTRACTS,
        UPCOMING_LABOUR_CONTRACTS.array[r].quantity, 20);
      remove_single_type_record(&UPCOMING_LABOUR_CONTRACTS, r);

    } else {
      r++;
    }

  }

  // Update existing contracts
  r=0;
  while (r<EXISTING_LABOUR_CONTRACTS.size) {

    // Reduce counter to contract end
    EXISTING_LABOUR_CONTRACTS.array[r].time--;

    // Contract ending
    if (EXISTING_LABOUR_CONTRACTS.array[r].time <= 0) {

      // Update records
      remove_single_type_record(&EXISTING_LABOUR_CONTRACTS, r);

    } else {
      r++;
    }

  }

  return 0;

}

int PD_update_deliveries() {

  int d=0;
  while (d<DIVISION_DELIVERIES.size) {

    // Reduce counter to arrival
    DIVISION_DELIVERIES.array[d].delivery_time--;

    // Check if delivery has arrived
    if (DIVISION_DELIVERIES.array[d].delivery_time <= 0) {

      // Add to correct working capital inventory
      for (int w=0; w<WORKING_CAPITAL_INVENTORY.size; w++) {

        // Update appropriate inventory
        if (WORKING_CAPITAL_INVENTORY.array[w].source_id ==
          DIVISION_DELIVERIES.array[d].source_id)
        {
          WORKING_CAPITAL_INVENTORY.array[w].quantity += DIVISION_DELIVERIES.array[d].quantity;
        }

      }

      // Remove the delivery from tracking
      remove_delivery(&DIVISION_DELIVERIES, d);

    } else {
      d++;
    }

  }

  return 0;

}

int PD_produce() {

  int e, w;

  // Calculate output potential given labour and fixed capital constraints
  int labour = 0;
  for (e=0; e<EXISTING_LABOUR_CONTRACTS.size; e++) {
    labour += EXISTING_LABOUR_CONTRACTS.array[e].quantity;
  }
  double effective_fixed_capital = ((double) labour) / LABOUR_PER_UNIT_FIXED_CAPITAL;
  int fixed_capital_potential = floor(effective_fixed_capital * DAILY_UNIT_FIXED_CAPITAL_OUTPUT);

  // Calculate output potential given working capital constraints
  int working_capital_potential = -1;
  for (w=0; w<WORKING_CAPITAL_INVENTORY.size; w++) {
    if (WORKING_CAPITAL_INVENTORY.array[w].quantity < working_capital_potential ||
      working_capital_potential == -1)
    {
      working_capital_potential = WORKING_CAPITAL_INVENTORY.array[w].quantity;
    }
  }

  // Work out overall output potential and produce
  int output = fixed_capital_potential < working_capital_potential ?
    fixed_capital_potential : working_capital_potential;
  CUMULATIVE_OUTPUT += output;

  // Adjust working inventory accordingly
  for (w=0; w<WORKING_CAPITAL_INVENTORY.size; w++) {
    WORKING_CAPITAL_INVENTORY.array[w].quantity -= output;
  }

  // Send off the output
  add_division_delivery_notification_message(FIRM_ID, DIVISION_ID, DELIVERS_TO_ID,
    output, TIME_TO_DELIVER);

  return 0;

}

int PD_add_delivery() {

  // Add deliveries
  START_DIVISION_DELIVERY_NOTIFICATION_MESSAGE_LOOP
    add_delivery(&DIVISION_DELIVERIES,
      division_delivery_notification_message->quantity,
      division_delivery_notification_message->delivery_time,
      division_delivery_notification_message->source_division_id);
  FINISH_DIVISION_DELIVERY_NOTIFICATION_MESSAGE_LOOP

  return 0;

}
