#include "header.h"
#include "Division_agent_header.h"

int produce() {

  // Check that there is a buyer, and that the current order has not been completed
  if (CURRENT_ORDER.buyer > -1 && CURRENT_ORDER.quantity > 0) {

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
    CURRENT_ORDER.buyer = -1;

  }

  return 0;

}

int deliver() {

  // Check an order is in progress and that stock exists to deliver
  if (CURRENT_ORDER.buyer > -1 && CURRENT_ORDER.quantity > 0 && STOCK > 0) {

    // Check how much can be transported
    if (STOCK > TRANSPORTABLE_DAILY) {
      TRANSPORT_QUANTITY = TRANSPORTABLE_DAILY;
    } else {
      TRANSPORT_QUANTITY = STOCK;
    }

    // Add the delivery message

    // add_delivery_message(PRODUCES, TRANSPORT_QUANTITY, X_POSITION, Y_POSITION);

  }

  return 0;

}
