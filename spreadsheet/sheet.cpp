#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::Sheet(){
    empty_cell_ = std::make_unique<Cell>(*this);
    empty_cell_.get()->Set("",*this);
}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {

    if (!pos.IsValid())
    {
        throw InvalidPositionException("invalid position");
    }
    else
    {

        auto cell = std::make_unique<Cell>(*this, pos);
        cell.get()->Set(text, *this);

        std::set<const CellInterface*> alredyChecked;

        if (!CheckCircularRef(pos, cell.get(),alredyChecked))
            throw CircularDependencyException("circular dependency");

        sheet_dependenses_.erase(pos);
        
        for(auto& position : cell->GetReferencedCells()){
            sheet_dependenses_[position].insert(pos);
        }

        sheet_[pos] = std::move(cell);


//        sheet_dependenses_.erase(pos);
//        for(auto& cell : sheet_){
//            for(auto& ref : cell.second->GetReferencedCells()){
//                if(ref == pos)
//                    sheet_dependenses_[pos].insert(cell.first);
//            }
//        }


        if(pos.row+1>print_size_.rows)
            print_size_.rows = pos.row+1;
        if(pos.col+1>print_size_.cols)
            print_size_.cols = pos.col+1;
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {

    if (!pos.IsValid())
        throw InvalidPositionException("invalid position");

    auto result = sheet_.find(pos);

    if (pos.row < print_size_.rows && pos.col < print_size_.cols) {
        if (result != sheet_.end()) {
            return result->second.get();
        }
        else {
            return empty_cell_.get();
        }
    }

    return nullptr;

}
CellInterface* Sheet::GetCell(Position pos) {

    if (!pos.IsValid())
        throw InvalidPositionException("invalid position");

    auto result = sheet_.find(pos);

    if (pos.row < print_size_.rows && pos.col < print_size_.cols) {
        if (result != sheet_.end()) {
            return result->second.get();
        }
        else {
            return empty_cell_.get();
        }
    }

    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid())
        throw InvalidPositionException("invalid position");

    auto res_row = sheet_.find(pos);
    if(res_row != sheet_.end()){
        sheet_.erase(res_row);
        RecalcPrintSize();

    }

    sheet_dependenses_.erase(pos);
    for(auto& cell : sheet_dependenses_)
    {
        if(cell.second.count(pos)>0)
            cell.second.erase(pos);
    }

}

Size Sheet::GetPrintableSize() const {
    return print_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < print_size_.rows; ++row) {
        for (int col = 0; col < print_size_.cols; ++col) {
            Position pos = {row, col};
            const CellInterface* cell = GetCell(pos);

            if(cell == nullptr){
                if (col != print_size_.cols - 1)
                    output << "\t"sv;
                continue;
            }

            if (std::holds_alternative<std::string>(cell->GetValue())) {
                output << std::get<std::string>(cell->GetValue());
            } else if (std::holds_alternative<double>(cell->GetValue())) {
                output << std::get<double>(cell->GetValue());
            } else if (std::holds_alternative<FormulaError>(cell->GetValue())) {
                output << std::get<FormulaError>(cell->GetValue());
            }

            if (col != print_size_.cols - 1)
                output << "\t"sv;

        }

        output << "\n"sv;
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < print_size_.rows; ++row) {
        for (int col = 0; col < print_size_.cols; ++col) {
            Position pos = { row, col };
            const CellInterface* cell = GetCell(pos);

            if (col != print_size_.cols - 1) {
                if(cell != nullptr)
                    output << cell->GetText();
                output << "\t"sv;
            }
            else {
                if(cell != nullptr)
                    output << cell->GetText();
            }
        }

        output << "\n"sv;
    }
}

void Sheet::RecalcPrintSize()
{
    if(sheet_.empty()){
        print_size_ = Size{0, 0};
        return;
    }

    print_size_ = Size{0, 0};

    for (auto& pos : sheet_) {
        if(pos.first.row >= print_size_.rows)
            print_size_.rows = pos.first.row + 1;
        if(pos.first.col >= print_size_.cols)
            print_size_.cols = pos.first.col + 1;
    }

}

std::set<Position> Sheet::GetDependCells(Position checkedCell)
{
    auto row = sheet_dependenses_.find(checkedCell);
    if(row != sheet_dependenses_.end())
    {
        return row->second;
    }
    return std::set<Position>{};
}


bool Sheet::CheckCircularRef(Position check_pos, const CellInterface *cell, std::set<const CellInterface *> &alredyChecked) const
{
    if (cell == nullptr)
        return true;

    std::vector<Position> refCells = cell->GetReferencedCells();

    auto findRes = alredyChecked.find(cell);
    if(findRes == alredyChecked.end())
    {
        for (const auto& pos : refCells) {
            if (check_pos == pos)
                return false;
            alredyChecked.insert(cell);
            if (!CheckCircularRef(check_pos, GetCell(pos),alredyChecked))
                return false;
        }
    }

    return true;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
