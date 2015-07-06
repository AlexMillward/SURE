#include "header.h"
#include "PurchaseMarket_agent_header.h"

int PM_update_price() {

  // Calculate the change in sales
  int sales_change = LAST_SALES - CUMULATIVE_SALES;
  PRICE += ( ((double) sales_change) / ((double) PRICE_ELASTICITY) );

  // Update sales counters
  LAST_SALES = CUMULATIVE_SALES;
  CUMULATIVE_SALES = 0;

  return 0;

}

/*
int state_price() {

  add_market_price_message(MARKET_ID, PRICE);

  return 0;

}
*/
