/**
 * Limits the input value between the input min and input max
 * @param val the value to be limited
 * @param max the maximum output value
 * @param min the minimum output value
 * @return the limited value
 */
double limitValue(double val, double max, double min) {
    if(val > max) {
        return max;
    } else if(val < min) {
        return min;
    } else {
        return val;
    }
}

/**
 *  Scales the value in to the specified range
 * @param valueIn the value to be scaled
 * @param baseMin the minimum input value
 * @param baseMax the maximum input value
 * @param limitMin the scaled minimum output
 * @param limitMax the scaled maximum output
 * @return the scaled value
 */
double scale(double valueIn, double baseMin, double baseMax, double limitMin, double limitMax) {
    return ((limitMax - limitMin) * (valueIn - baseMin) / (baseMax - baseMin)) + limitMin;
}

/**
 * Scales the value in to the specified range and limits the output to the max and min specified
 * @param valueIn the value to be scaled
 * @param baseMin the minimum input value
 * @param baseMax the maximum input value
 * @param limitMin the scaled minimum output
 * @param limitMax the scaled maximum output
 * @return the scaled value
 */
double scaleClipped( double valueIn,  double baseMin,  double baseMax,  double limitMin,  double limitMax) {
    double calculatedValue = ((limitMax - limitMin) * (valueIn - baseMin) / (baseMax - baseMin)) + limitMin;
    return limitValue(calculatedValue, limitMax, limitMin);
}

/**
 * Squares the input value while keeping the +/- sign
 * @param val the value to be squared
 * @return the squared value
 */
double squareMaintainSign(double val) {
    double output = val * val;

    //was originally negative
    if(val < 0) {
        output = -output;
    }

    return output;
}

/**
 * Cubes the input value while keeping the +/- sign
 * @param val the value to be cubed
 * @return the cubed value
 */
double power3MaintainSign(double val){
    double output = val*val*val;
    return output;
}

/**
 * Returns the maximum value of the three passed in
 * @param a input 1
 * @param b input 2
 * @param c input 3
 * @return the max of a, b, c
 */
double maxOf3(double a, double b, double c) {
    a = abs(a);
    b = abs(b);
    c = abs(c);
    if(a > b && a > c) {
        return a;
    } else if(b > c) {
        return b;
    } else {
        return c;
    }
} 