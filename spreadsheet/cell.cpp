#include "cell.h"

#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

//class Impl{
//public:

//    virtual CellInterface::Value GetValue() const = 0;
//    virtual const std::string GetText() const = 0;
//    virtual std::vector<Position> GetReferencedCells() const = 0;
//    virtual void InvalidateCache() = 0;

//};

//class EmptyImpl: public Impl{
//public:
//    //EmptyImpl() = default;
//    CellInterface::Value GetValue() const override;
//    const std::string GetText() const override;
//    std::vector<Position> GetReferencedCells() const override;
//    void InvalidateCache() override;
//};

//class TextImpl: public Impl{
//public:
//    TextImpl(const std::string& text);
//    CellInterface::Value GetValue() const override;
//    const std::string GetText() const override;
//    std::vector<Position> GetReferencedCells() const override;
//    void InvalidateCache() override;
//private:
//    std::string text_;
//};

//class FormulaImpl: public Impl {
//public:
//    FormulaImpl(const std::string& text, const SheetInterface& sheet);
//    CellInterface::Value GetValue() const override;
//    const std::string GetText() const override;
//    std::vector<Position> GetReferencedCells() const override;
//    void InvalidateCache() override;

//private:
//    std::unique_ptr<FormulaInterface> formula_impl_ptr_;
//    //SheetInterface& sheetI_;
//    //Sheet& sheetI_;
//    const SheetInterface &sheetI_;
//    mutable std::optional<double> cache_ = std::nullopt;
//};

// Реализуйте следующие методы
Cell::Cell(Sheet &sheet)
    :impl_(std::make_unique<EmptyImpl>())
    ,sheet_(sheet)
{

}

Cell::Cell(Sheet &sheet, Position pos)
    :impl_(std::make_unique<EmptyImpl>())
    ,sheet_(sheet)
    ,pos_(pos)
{}

Cell::~Cell() {}

void Cell::Set(std::string text, const SheetInterface& sheet)
{
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
        return;
    }


    if(text.front() ==  FORMULA_SIGN && static_cast<int>(text.size()) > 1)
    {
        //impl_ = std::make_unique<FormulaImpl>(text.substr(1), sheet);
        impl_ = std::make_unique<FormulaImpl>(text, sheet);
    }
    else
    {
        impl_ = std::make_unique<TextImpl>(text);
    }


    InvalidateCache(pos_);

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

std::vector<Position> Cell::GetDependentCells() const
{
    std::vector<Position> result;
    for(auto pos : sheet_.GetDependCells(pos_))
        result.push_back(pos);
    return result;
}

void Cell::InvalidateCache(Position pos)
{
    CellInterface* cell = sheet_.GetCell(pos);
    if(cell != nullptr)
        dynamic_cast<Cell*>(sheet_.GetCell(pos))->InvalidateCache();
    //cell->InvalidateCache();

    for(auto& depCells : sheet_.GetDependCells(pos)){
        InvalidateCache(depCells);
    }
}

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
    std::string test;
    test = "ERR";
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



