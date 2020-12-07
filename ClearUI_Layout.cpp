#include "ClearUI_Layout.h"


bool Layout::render(bool refresh) {
  refresh |= this->Field::render(refresh);

  bool updated = refresh;

  for (auto&& f : fields) {
    updated |= f->render(refresh);
  }

  return updated;
}

void Layout::select(bool s) {
  if (focus == focusField) {
    selectedField()->select(s);
  }
}

void Layout::enter(bool alternate) {
  focus = focusNavigate;
  selectedField()->select();
}

void Layout::exit() {
  if (focus == focusField)
    selectedField()->exit();

  selectedField()->deselect();
  focus = focusNone;
}

bool Layout::click(Button::State s) {
  switch (focus) {

    case focusNone:
      focus = focusNavigate;
      selectedField()->select();
      // fall through;

    case focusNavigate:
      switch (s) {
        case Button::DownLong:
          focus = focusField;
          selectedField()->enter(true);
          break;
        case Button::Up:
          focus = focusField;
          selectedField()->enter(false);
          break;
        default:
          break;
      }
      break;

    case focusField:
      if (selectedField()->click(s)) {
        break;
      }
      switch (s) {
        case Button::DownLong:
          // held down after selecting... so exit and re-enter
          focus = focusNavigate;
          selectedField()->exit();
          selectedField()->enter(true);
          break;
        case Button::Up:
        case Button::UpLong:
          focus = focusNavigate;
          selectedField()->exit();
          break;
        default:
          break;
      }
      break;
  }
  return true;  // hmmmm... is this right?
}

void Layout::update(Encoder::Update u) {
  switch (focus) {

    case focusNone:
      focus = focusNavigate;
      selectedField()->select();
      break;

    case focusNavigate:
      selectedField()->deselect();
      selectedIndex =
        constrain(selectedIndex + u.dir(),
          0, (int)(fields.size()) - 1);
      selectedField()->select();
      break;

    case focusField:
      selectedField()->update(u);
      break;
  }

}

Field* Layout::selectedField() const {
  return fields.begin()[selectedIndex];
}

void Layout::redraw() {
  // nothing by default
}


void Frame::show(Field* f) {
  if (f != content) {
    exit();
    content = f;
    enter(false);
  }
}

bool Frame::render(bool force) {
  bool drew = this->Field::render(force);
  drew |= content ? content->render(drew) : false;
  drawnContent = content;
  return drew;
}

void Frame::select(bool s)             {    if (content)  content->select(s); }
void Frame::enter(bool a)              {    if (content)  content->enter(a); }
void Frame::exit()                     {    if (content)  content->exit(); }
bool Frame::click(Button::State s)     { return content ? content->click(s) : false; }
void Frame::update(Encoder::Update u)  {    if (content)  content->update(u); }

bool Frame::isOutOfDate() { return content != drawnContent; }
void Frame::redraw() { }
