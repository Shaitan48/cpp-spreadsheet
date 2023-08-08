#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text) ;
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

private:
//можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl{
        using Value = std::variant<std::monostate, std::string, std::unique_ptr<FormulaInterface>>;
    public:
        Value& GetChangedValue() {
            return _value;
        }

        const Value& GetValue() const {
            return _value;
        }

    private:
        Value _value;

    };
    class EmptyImpl: public Impl{
    public:
        EmptyImpl() = default;
    };
    class TextImpl: public Impl{
    public:
        TextImpl(const std::string& text) {
            GetChangedValue() = text;
        }
    };
    class FormulaImpl: public Impl {
    public:
        FormulaImpl(const std::string& text) {
            GetChangedValue() = ParseFormula(text);
        }
    };
    std::unique_ptr<Impl> _impl;

};
