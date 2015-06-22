#include "Division_helpers.h"

int compare_availability_costs(const void * a, const void * b) {

  return ( ((division_availability_information*) a)->unit_cost -
    ((division_availability_information*) b)->unit_cost);

}
