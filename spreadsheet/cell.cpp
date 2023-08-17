#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(Sheet &sheet)
    :impl_(std::make_unique<EmptyImpl>())
    ,sheet_(sheet)
{}

Cell::~Cell() {}

void Cell::Set(std::string text, const SheetInterface& sheet)
{
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
        return;
    }

    if(text.front() ==  ESCAPE_SIGN)
    {
        impl_ = std::make_unique<TextImpl>(text);
    }else
    {
        if(text.front() ==  FORMULA_SIGN && static_cast<int>(text.size()) > 1)
        {
            //impl_ = std::make_unique<FormulaImpl>(text.substr(1), sheet);
            impl_ = std::make_unique<FormulaImpl>(text, sheet);
        }
        else
        {
            impl_ = std::make_unique<TextImpl>(text);
        }
    }

    void InvalidateCache();

}

void Cell::Clear() {
    impl_.reset(nullptr);
    //impl_ = std::make_unique<EmptyImpl>();
}

/*
//Cell::Value Cell::GetValue() const {
//    if (std::holds_alternative<std::string>(impl_.get()->GetValue()))
//    {
//        std::string res = std::get<std::string>(impl_.get()->GetValue());
//        if (res.empty())
//        {
//            return {};
//        }

//        if (res.front() == ESCAPE_SIGN)
//        {
//            return res.substr(1);
//        }

//        return res;
//    }
//    if (std::holds_alternative<std::unique_ptr<FormulaInterface>>(impl_.get()->GetValue()))
//    {
//        std::variant<double, FormulaError> res;
//        res = std::get<std::unique_ptr<FormulaInterface>>(impl_.get()->GetValue())->Evaluate();

//        if (std::holds_alternative<double>(res))
//        {
//            return std::get<double>(res);
//        }

//        if (std::holds_alternative<FormulaError>(res))
//            return std::get<FormulaError>(res);
//    }
//    return {};
//}
*/

Cell::Value Cell::GetValue() const  {
    return impl_->GetValue();
}

std::string Cell::GetText() const
{
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const
{
    return impl_->GetReferencedCells();
}

//bool Cell::CheckCircular(const Position check_pos, const CellInterface* cell) const
//{
//    if (cell == nullptr)
//        return true;

//    std::vector<Position> ref_cell_pos = cell->GetReferencedCells();

//    if (ref_cell_pos.empty())
//        return true;

//    for (const auto& pos : ref_cell_pos) {
//        if (check_pos == pos)
//            return false;
//        //вот тут не вышло немного.. не смог из sheet_ вытащить const CellInterface* придется переностиь в sheet.cpp
//          reinterpret_cast вроде как не фэншуй
//          if (!CheckCircular(check_pos, sheet_)
//            return false;
//    }

//    return true;
//}

void Cell::InvalidateCache()
{
    impl_->InvalidateCache();
}



CellInterface::Value Cell::EmptyImpl::GetValue() const
{
    return "";
}

const std::string Cell::EmptyImpl::GetText() const
{
    return "";
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const
{
    return std::vector<Position>{};
}

void Cell::EmptyImpl::InvalidateCache()
{

}

Cell::TextImpl::TextImpl(const std::string &text):text_(text){}

CellInterface::Value Cell::TextImpl::GetValue() const
{
    if (text_.empty()) {
        throw FormulaException("not text");
    }

    if (text_.front() == ESCAPE_SIGN)
    {
        return text_.substr(1);
    }

    return text_;
}

const std::string Cell::TextImpl::GetText() const
{
    return text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const
{
    return std::vector<Position>{};
}

void Cell::TextImpl::InvalidateCache()
{

}

Cell::FormulaImpl::FormulaImpl(const std::string &text, const SheetInterface &sheet)
    :sheetI_(sheet)
{
    formula_impl_ptr_ = ParseFormula(text.substr(1));
}

CellInterface::Value Cell::FormulaImpl::GetValue() const
{
    if(cache_.has_value())
        return cache_.value();

    auto res = formula_impl_ptr_->Evaluate(sheetI_);

    if (std::holds_alternative<double>(res))
    {
        cache_.emplace(std::get<double>(res));
        return std::get<double>(res);
    }

    return std::get<FormulaError>(res);
}

const std::string Cell::FormulaImpl::GetText() const
{
    return FORMULA_SIGN + formula_impl_ptr_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const
{
    return formula_impl_ptr_->GetReferencedCells();
}

void Cell::FormulaImpl::InvalidateCache()
{
    cache_ = std::nullopt;
}



