#ifndef _INCLUDE_CLEARUI_FIELD_H_
#define _INCLUDE_CLEARUI_FIELD_H_

#include <algorithm>
#include <initializer_list>
#include <stdint.h>

#include "ClearUI_Display.h"
#include "ClearUI_Input.h"

/**
 **  Field
 **/

class Field {
public:
  Field(int16_t x, int16_t y, uint16_t w, uint16_t h)
    : x(x), y(y), w(w), h(h),
      selected(false), selectedAsDrawn(false)
    { };

  virtual bool render(bool refresh);
  inline bool render() { return render(false); };

  virtual void select(bool s);
  inline void select() { select(true); };
  inline void deselect() { select(false); };

  virtual void enter(bool alternate);
  virtual void exit();

  virtual bool click(Button::State s);
  virtual void update(Encoder::Update);

protected:
  virtual bool isOutOfDate();
  inline bool isSelected() { return selected; }

  inline uint16_t foreColor() { return selected ? BLACK : WHITE; };
  inline uint16_t backColor() { return selected ? WHITE : BLACK; };

  virtual void redraw() = 0;

  const     int16_t   x, y;
  const     uint16_t  w, h;

private:
  bool selected;

  bool selectedAsDrawn;
};


/**
 **  OptionField
 **/

template< typename T >
class OptionField : public Field {
public:
  typedef T value_t;

  OptionField(
      int16_t x, int16_t y, uint16_t w, uint16_t h,
      const std::initializer_list<value_t>& options
      )
    : Field(x, y, w, h),
      options{},
      numOptions(min(maxOptions, (int)(options.size())))
    {
      std::copy(
        options.begin(), options.begin() + numOptions,
        const_cast<value_t*>(this->options));
    };

  virtual void select(bool);
  virtual void update(Encoder::Update);

  virtual value_t getValue() = 0;
  virtual void    setValue(const value_t& v) = 0;

protected:
  int findOptionIndex(const value_t&);

  static const int maxOptions = 16;
  const value_t    options[maxOptions];
  const int        numOptions;

private:
  value_t   entryValue;
  bool      entryValueIsOption;
};

template< typename T >
void OptionField<T>::select(bool s) {
  Field::select(s);
  if (s) {
    entryValue = getValue();
    entryValueIsOption = findOptionIndex(entryValue) >= 0;
  }
}

template< typename T >
int OptionField<T>::findOptionIndex(const value_t& v) {
  for (int i = 0; i < numOptions; i++) {
    if (v == options[i])
      return i;
  }
  return -1;
}

template< typename T >
void OptionField<T>::update(Encoder::Update update) {
  int i = findOptionIndex(getValue());
  i = constrain(i + update.dir(), -1, numOptions - 1);

  if (i >= 0) {
    setValue(options[i]);
  } else if (entryValueIsOption) {
    setValue(options[0]);
  } else {
    setValue(entryValue);
  }
}


/**
 **  ValueField
 **/

template< typename T >
class ValueField : public OptionField<T> {
public:
  ValueField(
      int16_t x, int16_t y, uint16_t w, uint16_t h,
      T& value, const std::initializer_list<T>& options
      )
    : OptionField<T>(x, y, w, h, options), value(value), valueAsDrawn(value)
      { };

  virtual T    getValue()           { return value; }
  virtual void setValue(const T& v) { value = v; }

protected:
  virtual bool isOutOfDate()
    { return valueAsDrawn != value || OptionField<T>::isOutOfDate(); }
  virtual void redraw();

  T& value;
  T valueAsDrawn;
};

template< typename T >
void ValueField<T>::redraw() {
  display.setTextColor(this->foreColor());
  centerNumber(value, this->x, this->y, this->w, this->h);
  valueAsDrawn = value;
}

/**
 **  PairField
 **/

template< typename T, typename U >
class PairField : public OptionField< std::pair<T, U> > {
public:
  typedef std::pair<T, U> value_t;

  PairField(
      int16_t x, int16_t y, uint16_t w, uint16_t h,
      ValueField<T>& fieldA, ValueField<U>& fieldB,
      const std::initializer_list<value_t>& options
      )
    : OptionField<value_t>(x, y, w, h, options),
      fieldA(fieldA), fieldB(fieldB)
    { }

  virtual void select(bool);

protected:
  virtual value_t getValue()
    { return std::make_pair(fieldA.getValue(), fieldB.getValue()); }

  virtual void setValue(const value_t& v)
    { fieldA.setValue(v.first); fieldB.setValue(v.second); }

  virtual void redraw() { };

  ValueField<T>& fieldA;
  ValueField<T>& fieldB;
};

template< typename T, typename U >
void PairField<T, U>::select(bool s) {
  fieldA.select(s);
  OptionField<value_t>::select(s);
  fieldB.select(s);
}

#endif // _INCLUDE_CLEARUI_FIELD_H_
