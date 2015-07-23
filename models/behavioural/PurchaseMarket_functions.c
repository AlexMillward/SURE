#include "header.h"
#include "PurchaseMarket_agent_header.h"

int PM_update_price() {

  if (iteration_loop % 20 == 1) {
    // Calculate the change in sales
    int sales_change = LAST_SALES - CUMULATIVE_SALES;
    PRICE += ( ((double) sales_change) / ((double) PRICE_ELASTICITY) );
    // Update sales counters
    LAST_SALES = CUMULATIVE_SALES;
    CUMULATIVE_SALES = 0;
  }

  return 0;

}

int PM_state_price() {

  // Send out the price to potential buyers
  add_market_price_message(MARKET_ID, PRICE);

  return 0;

}

int PM_order_confirmation() {

  START_MARKET_ORDER_MESSAGE_LOOP

    // Confirm order
    add_market_order_confirmation_message(MARKET_ID,
      market_order_message->global_id,
      market_order_message->internal_id,
      PRICE, market_order_message->quantity);

  FINISH_MARKET_ORDER_MESSAGE_LOOP

  return 0;

}
