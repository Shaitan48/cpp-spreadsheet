#pragma once

#include "common.h"
#include "formula.h"
#include <optional>

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text) ;
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;
    std::optional<double>& GetCache() const;
    bool IsReferenced() const;

private:
//можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl{
        //using Value = std::variant<std::monostate, std::string, std::unique_ptr<FormulaInterface>>;
    public:

        virtual Value GetValue() const = 0;
        virtual const std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;


    };
    class EmptyImpl: public Impl{
    public:
        EmptyImpl() = default;
        Value GetValue() const override;
        const std::string GetText() const override;
    };
    class TextImpl: public Impl{
    public:
        TextImpl(const std::string& text):text_(text){};
        Value GetValue() const override;
        const std::string GetText() const override;
    private:
        std::string text_;
    };
    class FormulaImpl: public Impl {
    public:
        FormulaImpl(const std::string& text);

        Value GetValue() const override;
        const std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;

    private:
        std::unique_ptr<FormulaInterface> formula_impl_ptr_;
    };

    void InvalidateCache();

private:
    std::unique_ptr<Impl> impl_;
    mutable std::optional<double> cache_;

};
