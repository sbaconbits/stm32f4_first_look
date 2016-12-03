#define CLR_BITS(_reg, _mask, _shift) \
    _reg &= ~(_mask << _shift)

#define SET_BITS(_reg, _val, _shift) \
    _reg |= (_val << _shift)

#define SET_BITS_AFTER_CLR(_reg, _mask, _val, _shift)  \
    CLR_BITS(_reg, _mask, _shift);                      \
    SET_BITS(_reg, _val,  _shift);


