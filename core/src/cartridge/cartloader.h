#pragma once

#include "cartridge/cart.h"
#include "cartridge/cartkind.h"
#include "nesish/nesish.h"

NHErr
cartld_LoadCart(const char *rompath, cartkind_e kind, cart_s *cart,
                NHLogger *logger);
