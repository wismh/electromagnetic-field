#pragma once

namespace game {

struct FieldLayers {
    bool potential = true;
    bool lines = true;
    bool grid = false;
    bool flow = true;
    bool probe = true;
    bool trails = true;
    bool magnetic = false;
};

}
