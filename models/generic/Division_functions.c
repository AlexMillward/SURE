#include <math.h>
#include "header.h"
#include "Division_agent_header.h"

int produce() {

  // Check that there is a buyer, and that the current order has not been completed
  if (CURRENT_ORDER.buyer_id > -1 && CURRENT_ORDER.quantity > 0) {

    // Check dependencies to see how much can be made
    DEPENDENCY_OUTPUT_POTENTIAL = -1;
    for (int d=0; d<DEPENDENCIES.size; d++) {
      if (DEPENDENCIES.array[d].required_per_unit != 0) {
        int current_output_potential = DEPENDENCIES.array[d].stock / DEPENDENCIES.array[d].required_per_unit;
        if (d == 0 || current_output_potential < DEPENDENCY_OUTPUT_POTENTIAL) {
          DEPENDENCY_OUTPUT_POTENTIAL = current_output_potential;
        }
      }
    }

    // Check employee and capital output potential to see how much can be made
    EMPLOYEE_OUTPUT_POTENTIAL = EMPLOYEES * OUTPUT_POTENTIAL_PER_EMPLOYEE;
    CAPITAL_OUTPUT_POTENTIAL = QUANTITY_OF_CAPITAL * OUTPUT_POTENTIAL_PER_CAPITAL_UNIT;

    // Determine output potential
    OUTPUT_POTENTIAL = EMPLOYEE_OUTPUT_POTENTIAL;
    if (CAPITAL_OUTPUT_POTENTIAL < OUTPUT_POTENTIAL) {
      OUTPUT_POTENTIAL = CAPITAL_OUTPUT_POTENTIAL;
    }
    if (PRODUCTION_COSTS_PER_UNIT != 0) {
      // Checking for funds limitation
      FUNDS_OUTPUT_POTENTIAL = (FUNDS - RESERVED_FUNDS) / PRODUCTION_COSTS_PER_UNIT;
      if (FUNDS_OUTPUT_POTENTIAL < OUTPUT_POTENTIAL) {
        OUTPUT_POTENTIAL = FUNDS_OUTPUT_POTENTIAL;
      }
    } else {
      FUNDS_OUTPUT_POTENTIAL = -1;
    }
    if (DEPENDENCY_OUTPUT_POTENTIAL != -1 && DEPENDENCY_OUTPUT_POTENTIAL < OUTPUT_POTENTIAL) {
      OUTPUT_POTENTIAL = DEPENDENCY_OUTPUT_POTENTIAL;
    }

    // Determine actual output
    if (CURRENT_ORDER.quantity < OUTPUT_POTENTIAL) {
      OUTPUT = CURRENT_ORDER.quantity;
    } else {
      OUTPUT = OUTPUT_POTENTIAL;
    }

    // Adjust stock of dependencies for output
    for (int d=0; d<DEPENDENCIES.size; d++) {
      DEPENDENCIES.array[d].stock -= (DEPENDENCIES.array[d].required_per_unit * OUTPUT);
    }

    // Adjust funds based on production and add appropriate amount to stock
    FUNDS -= (PRODUCTION_COSTS_PER_UNIT * OUTPUT);
    STOCK += OUTPUT;

  } else {

    // Current order has depleted, or is already completed, so ensure it is completed
    CURRENT_ORDER.buyer_id = -1;

  }

  return 0;

}

int deliver() {

  // Check an order is in progress and that stock exists to deliver
  if (CURRENT_ORDER.buyer_id > -1 && CURRENT_ORDER.quantity > 0 && STOCK > 0) {

    // Check how much can be transported
    if (STOCK > TRANSPORTABLE_DAILY) {
      TRANSPORT_QUANTITY = TRANSPORTABLE_DAILY;
    } else {
      TRANSPORT_QUANTITY = STOCK;
    }

    // Add the delivery message if any units exist to deliver
    if (TRANSPORT_QUANTITY > 0) {
      add_delivery_message(CURRENT_ORDER.buyer_id, PRODUCES_GOOD_OF_TYPE, TRANSPORT_QUANTITY, X_POSITION, Y_POSITION);
    }

  }

  return 0;

}

int handleDeliveries() {

  // Iterate through delivery messages
  START_DELIVERY_MESSAGE_LOOP

    // If the message being considered is intended for this division, save the delivery and compute the delivery time
    if (ID == delivery_message->destination_id) {
      add_delivery(&DELIVERIES, delivery_message->good_type, delivery_message->quantity,
        ceil(sqrt(
          pow(delivery_message->origin_x - X_POSITION, 2) +
          pow(delivery_message->origin_y - Y_POSITION, 2)
        ))
      );
    }

  FINISH_DELIVERY_MESSAGE_LOOP

  return 0;

}

int updateDeliveries() {

  // Initialise iteration
  int d = 0;
  while (d < DELIVERIES.size) {

    // Reduce time remaining
    DELIVERIES.array[d].time_remaining--;

    // Check if the delivery has arrived, then adjust iteration accordingly
    if (DELIVERIES.array[d].time_remaining <= 0) {

      // Find dependency for the good type of the delivery and adjust its stock
      for (int e=0; e<DEPENDENCIES.size; e++) {
        if (DEPENDENCIES.array[e].good_type == DELIVERIES.array[d].good_type) {
          DEPENDENCIES.array[e].stock += DELIVERIES.array[d].quantity;
          break;
        }
      }

      // Remove the delivery
      remove_delivery(&DELIVERIES, d);

    } else {
      d++;
    }

  }

  return 0;

}

int stateInformation() {

  // Retrieve dependencies
  int dependency_ids[5];
  for (int d=0; d<5; d++) {
    dependency_ids[d] = (d < DEPENDENCIES.size) ? DEPENDENCIES.array[d].good_type : -1;
  }

  // Find costs
  RUNNING_COSTS = MEAN_WAGE * EMPLOYEES;
  UNIT_COSTS = PRODUCTION_COSTS_PER_UNIT;

  // Send the information
  add_division_information_message(FIRM_ID, ID, PRODUCES_GOOD_OF_TYPE,
    RUNNING_COSTS, UNIT_COSTS, dependency_ids);

  return 0;

}
