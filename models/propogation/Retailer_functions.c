#include <math.h>
#include "header.h"
#include "Retailer_agent_header.h"
#include "Division_helpers.h"

int restock() {

  // Variable initialisation
  int p, r, depletion_time;
  retailer_requirement_array requirements;
  init_retailer_requirement_array(&requirements);

  for (p=0; p<RETAILER_PRODUCTS.size; p++) {

    // Calculate the time in which stock will deplete
    depletion_time = (RETAILER_PRODUCTS.array[p].stock + RETAILER_PRODUCTS.array[p].ordered) /
      RETAILER_PRODUCTS.array[p].units_sold_per_day;

    // Variable initialisation
    firm_cost_information cost_information;
    init_firm_cost_information(&cost_information);

    // Check if restock is required, if so add requirement
    if (depletion_time < RETAILER_PRODUCTS.array[p].restock_depletion_time) {
      add_retailer_requirement(&requirements, p, depletion_time,
        -1, -1, -1, -1, &cost_information, -1);
    }

  }

  // Variable initialisation
  retailer_product * current_product;
  retailer_requirement * current_requirement;
  division_transport_quote transport_quote;
  double current_price;

  START_FIRM_AVAILABILITY_MESSAGE_LOOP

    for (r=0; r<requirements.size; r++)  {

      // Assign pointers to correct values
      current_requirement = &requirements.array[r];
      current_product = &RETAILER_PRODUCTS.array[current_requirement->product_number];

      // Check that the correct good is being considered
      if (current_product->good_id == firm_availability_message->good_id)
      {

        // Get the unit price and delivery time
        transport_quote = calculate_transport_costs(
          &firm_availability_message->transport_information, X_POSITION, Y_POSITION, 1);
        current_price = firm_availability_message->price + transport_quote.cost;

        /*
          Check that:
            a) If on first iteration (best must be current)
            b) If the best delivery time is acceptable, whether the best price has been found
            c) If the best delivery time isn't acceptable, whether the current time beats it
        */
        if (current_requirement->best_firm_id == -1 ||
          (current_requirement->best_delivery_time <= current_requirement->depletion_time &&
          current_price < current_requirement->best_price) ||
          (current_requirement->best_delivery_time > current_requirement->depletion_time &&
          transport_quote.time < current_requirement->best_delivery_time))
        {

          // Update the current requirement with the best offer that has been found
          current_requirement->best_firm_id = firm_availability_message->firm_id;
          current_requirement->best_division_number = firm_availability_message->division_number;
          current_requirement->best_price = firm_availability_message->price;
          current_requirement->best_quantity = firm_availability_message->quantity;
          current_requirement->best_delivery_time = transport_quote.time;

        }

        // Clean up for iteration
        free_division_transport_quote(&transport_quote);

      }

    }

  FINISH_FIRM_AVAILABILITY_MESSAGE_LOOP

  // Variable initialisation
  double adjusted_price;
  int quantity_demanded, target_quantity, order_quantity;

  for (r=0; r<requirements.size; r++) {

    // Check a match was actually found
    if (current_requirement->best_firm_id != -1) {

      // Set up pointers
      current_requirement = &requirements.array[r];
      current_product = &RETAILER_PRODUCTS.array[current_requirement->product_number];

      // Adjust price for profit target
      adjusted_price = current_requirement->best_price *
        ((100 + current_product->percentage_profit_target) / 100);

      // Find the quantity demanded
      quantity_demanded = (adjusted_price - current_product->zero_demand_price) /
        current_product->price_elasticity_of_demand;

      // Find the desired quantity
      target_quantity = (quantity_demanded * current_product->target_depletion_time)
        - current_product->stock;

      // Find the amount that should actually be ordered
      order_quantity = target_quantity < current_requirement->best_quantity ?
        target_quantity : current_requirement->best_quantity;

      // Send the order
      add_product_order_message(
        current_requirement->best_firm_id,
        current_requirement->best_division_number,
        ID,
        current_requirement->product_number,
        current_requirement->best_price,
        order_quantity,
        current_requirement->best_delivery_time);

    }

  }

  // Clean up
  for (r=0; r<requirements.size; r++) {
    free_firm_cost_information(&requirements.array[r].best_cost_information);
  }
  free_retailer_requirement_array(&requirements);

  return 0;

}
