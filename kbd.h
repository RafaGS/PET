#pragma once

class kbd: public matrix_keyboard {
public:
    kbd(): _row(0), _ctrl(false), _shift(false), _ext(false), _super(false) {}

    void up(uint8_t);
    void down(uint8_t);
    void reset();

    void write(uint8_t r) { _row = r; }
    uint8_t read() { return _rows[_row] ^ 0xff; }
    uint8_t row() { return _row; }
    bool super_held() const { return _super; }

private:
    uint8_t _map(uint8_t);
    void _set(uint8_t);
    void _reset(uint8_t);

    uint8_t _rows[10];
    uint8_t _row;
    bool _ctrl, _shift, _ext, _super;
};
