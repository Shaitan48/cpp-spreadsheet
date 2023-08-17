#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <map>

class Sheet : public SheetInterface {
public:
    Sheet();

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    // Можете дополнить ваш класс нужными полями и методами
    void recalcPrintSize();

private:
    // Можете дополнить ваш класс нужными полями и методами
    //счачала строки, потом слолбцы
    struct PositionHasher
    {
        std::size_t operator()(const Position& pos) const noexcept
        {
            return std::hash<int>{}(pos.col + pos.row * Position::MAX_ROWS);
        }
    };

    std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> sheet_;
    Size print_size_ = Size{0, 0};


    bool CheckCircularRef(Position check_pos, const CellInterface* cell) const;



    std::unique_ptr<Cell> empty_cell_;
};
