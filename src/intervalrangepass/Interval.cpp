#include <set>
#include "Interval.h"

using namespace interval;

// Simple builder and accessor for pair representation
Interval interval::make(int l, int r) { return std::make_pair(l,r); }
int interval::lower(Interval i) { return i.first; }
int interval::upper(Interval i) { return i.second; }

// Pre-defined intervals
Interval interval::full() { return make(minf,pinf); }
Interval interval::empty() { return make(pinf,minf); }
Interval interval::unit() { return make(0,1); }

/* Least Upper Bound
 *   Edge cases lead to extreme intervals or extreme bounds.
 *   General case takes the lowest of the lows and the highest of the highs
 *   as the bounds.
 */
Interval interval::lub(Interval l, Interval r) {
  Interval result;
  if (l == full()) {
    result = full();
  } else if (l == empty()) {
    result = r;
  } else if (lower(l) == minf && upper(r) == pinf) {
    result = full();
  } else if (lower(l) == minf) {
    result = make(minf, std::max(upper(l), upper(r)));
  } else if (upper(l) == pinf) {
    result = make(std::min(lower(l), lower(r)), pinf);
  } else {
    result = make(std::min(lower(l), lower(r)), std::max(upper(l), upper(r)));
  }
  return result;
}

/* Unary negation
 *  Numerous special cases where the extreme bounds are involved.
 *  General case is to negate the bounds and use min/max to establish bounds.
 */
Interval interval::neg(Interval i) {
  Interval result;
  if (minf == lower(i) && pinf == upper(i)) {
    result = full();
  } else if (pinf == lower(i) && minf == upper(i)) {
    result = empty();
  } else if (minf == lower(i) && minf == upper(i)) {
    result = make(pinf, pinf);
  } else if (pinf == lower(i) && pinf == upper(i)) {
    result = make(minf, minf);
  } else if (pinf == upper(i)) {
    result = make(minf, -(lower(i)));
  } else if (minf == lower(i)) {
    result = make(-(upper(i)), pinf);
  } else {
    result = make(std::min(-(upper(i)),-(lower(i))),
                  std::max(-(upper(i)),-(lower(i))));
  }
  return result;
}

/* Addition
 *  Edge cases for empty intervals and maximal bounds
 *  General case is to add the corresponding bounds.
 */
Interval interval::add(Interval l, Interval r) {
  int low, up;

  if (pinf == lower(l) || pinf == lower(r)) {
    low = pinf; // one of the arguments is empty
  } else if (minf == lower(l) || minf == lower(r)) {
      low = minf;
  } else {
      //TODO: add overflow
    if(lower(l) > 0 && lower(r) > 0 && lower(r) > pinf - lower(l)){
        low = pinf;
    }
    else if(lower(l) < 0 && lower(r) < 0 && lower(r) < minf - lower(l)){
        low = minf;
    }
    else{
        low = lower(l) + lower(r);
    }
  }

  if (minf == upper(l) || minf == upper(r)) {
    up = minf; // one of the arguments is empty
  } else if (pinf == upper(l) || pinf == upper(r)) {
    up = pinf;
  } else {
      if(upper(l) > 0 && upper(r) > 0 && upper(r) > pinf - upper(l)){
          up =  pinf;
      }
      else if(upper(l) < 0 && upper(r) < 0 && upper(r) < minf - upper(l)){
          up = minf;
      }
      else{
          up = upper(l) + upper(r);
      }
  }

  return make(low, up);
}

Interval interval::sub(Interval l, Interval r) {
  return interval::add(l, interval::neg(r));
}

/* Multiplication
 */

int overflowHandler(double x, double y){
    double r = x * y;
    if (x > 0 && y > 0 && x > pinf / y){
        return pinf;
    }
    else if (x > 0 && y < 0 && x < minf / y){
        std::cerr << "smth" << std::endl;
        return minf;
    }
    else if (x < 0 && y > 0 && x < minf / y){
        std::cerr << "smth2" << std::endl;
        return minf;
    }
    else if (x < 0 && y < 0 && x > pinf / y){
        return pinf;
    }
    else{
        return (int)r;
    }
}

Interval interval::mul(Interval l, Interval r) {
    int low, up;

    if (pinf == lower(l) || pinf == lower(r)) {
        low = pinf; // one of the arguments is empty
    } else if (minf == lower(l) || minf == lower(r)) {
        low = minf;
    } else {

        low = std::min({overflowHandler(lower(l), lower(r)),
                overflowHandler(lower(l), upper(r)),
                overflowHandler(upper(l), lower(r)),
                overflowHandler(upper(l), upper(r))});
        std::cerr << "low is " << low << std::endl;
    }

    if (minf == upper(l) || minf == upper(r)) {
        up = minf; // one of the arguments is empty
    } else if (pinf == upper(l) || pinf == upper(r)) {
        up = pinf;
    } else {
        up = std::max({overflowHandler(lower(l), lower(r)),
                      overflowHandler(lower(l), upper(r)),
                      overflowHandler(upper(l), lower(r)),
                      overflowHandler(upper(l), upper(r))});
    }

    return make(low, up);
}

Interval multiHelper(Interval l, std::pair<double, double> r) {
    int low, up;
    double rl = r.first;
    double rr = r.second;

    low = std::min({overflowHandler(lower(l), rl),
                    overflowHandler(lower(l), rr),
                    overflowHandler(upper(l), rl),
                    overflowHandler(upper(l), rr)});

    up = std::max({overflowHandler(lower(l), rl),
                   overflowHandler(lower(l), rr),
                   overflowHandler(upper(l), rl),
                   overflowHandler(upper(l), rr)});


    return make(low, up);
}

/* Division
 */
Interval interval::div(Interval l, Interval r) {
    int y1 = lower(r);
    int y2 = upper(r);

    if(y1 < 0 && y2 > 0){
        //TODO: what to do with two intervals
        //TODO: hint in clion is not working
        return multiHelper(l, std::make_pair(minf, pinf));
    }
    else if(y1 == 0){
        return multiHelper(l, std::make_pair(1.0 / y2, pinf));
    }
    else if(y2 == 0){
        return multiHelper(l, std::make_pair(minf, 1.0 / y1));
    }
    else{
        return multiHelper(l, std::make_pair(1.0 / y2, 1.0 / y1));
    }
}

/* Comparison Operators
 *   Trivial imprecise definitions
 */
//TODO: comparsion for extra credit
Interval interval::lt(Interval l, Interval r) { return unit(); }
Interval interval::gt(Interval l, Interval r) { return unit(); }
Interval interval::eq(Interval l, Interval r) { return unit(); }
Interval interval::ne(Interval l, Interval r) { return unit(); }

std::string istr(int b) {
  std::string result = "";
  if (b == minf) {
    result = "-inf";
  } else if (b == pinf) {
    result = "+inf";
  } else {
    result = std::to_string(b);
  }
  return result;
}

std::string interval::str(Interval i) {
  std::string f = istr(lower(i));
  std::string s = istr(upper(i));
  return "[" + f + "," + s + "]";
}

// Deep equality for intervals
bool interval::operator==(Interval l, Interval r) {
  return (lower(l) == lower(r)) && (upper(l) == upper(r));
}

bool interval::operator!=(Interval l, Interval r) {
  return !(l == r);
}

