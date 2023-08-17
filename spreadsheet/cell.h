#pragma once

#include "common.h"
#include "formula.h"
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text, const SheetInterface &sheet) ;
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

//    bool CheckCircular(const Position check_pos, const CellInterface *cell) const;
    void InvalidateCache();


private:


    //можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl{
        //using Value = std::variant<std::monostate, std::string, std::unique_ptr<FormulaInterface>>;
    public:

        virtual Value GetValue() const = 0;
        virtual const std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual void InvalidateCache() = 0;

    };

    class EmptyImpl: public Impl{
    public:
        //EmptyImpl() = default;
        Value GetValue() const override;
        const std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        void InvalidateCache() override;
    };

    class TextImpl: public Impl{
    public:
        TextImpl(const std::string& text);
        Value GetValue() const override;
        const std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        void InvalidateCache() override;
    private:
        std::string text_;
    };

    class FormulaImpl: public Impl {
    public:
        FormulaImpl(const std::string& text, const SheetInterface& sheet);
        Value GetValue() const override;
        const std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        void InvalidateCache() override;

    private:
        std::unique_ptr<FormulaInterface> formula_impl_ptr_;
        //SheetInterface& sheetI_;
        //Sheet& sheetI_;
        const SheetInterface &sheetI_;
        mutable std::optional<double> cache_ = std::nullopt;
    };

private:
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
};
