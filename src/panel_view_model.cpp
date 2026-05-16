#include <game/panel_view_model.h>

#include <asset_ids.h>

namespace game {

PanelViewModel::PanelViewModel() {
    assets::ui::Panel::bind(*this);
}

}
