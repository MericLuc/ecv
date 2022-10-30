/**
 * @file ecv.hpp
 * @brief Main header of the ecv library
 * @author lhm
 */

#ifndef INCLUDE_ECV_HPP
#define INCLUDE_ECV_HPP

// Standard headers
#include <memory>
#include <stack>
#include <string>
#include <vector>

/*!
 * \brief Main namespace of the \a ecv library
 */
namespace ecv {

class DLX
{
public:
    typedef std::stack<int> Solution; ///< A solution is a combinaison of rows

    virtual std::vector<Solution>    solve(bool all = true) noexcept;
    virtual std::vector<std::string> toString(const Solution& s) noexcept = 0;

protected:
    DLX(const std::vector<bool>& data, size_t rows, size_t cols, const std::vector<int>& rowsList)
    noexcept;
    virtual ~DLX() noexcept = default;

protected:
    struct Impl;
    std::shared_ptr<Impl> pimpl{ nullptr };
};

/*!
 * \brief The DLX class is the DLX implementation of an exact cover problem
 * \see https://arxiv.org/pdf/cs/0011047v1.pdf for more informations about
 * DLX (dancing links) algorithm.
 */
class LatinSquares : public DLX
{
public:
    /*!
     * \brief generate Generate a data structure corresponding to the "latin squares"
     * exact cover problem.
     * For more informations about that problem \see https://en.wikipedia.org/wiki/Latin_square
     * \param inputs a String representation of the problem as a grid.
     * Use '0' to represent non-constrained cells
     * \return An exact cover problem pointer (\a Problem) in case of success, nullptr otherwise
     */
    static std::unique_ptr<LatinSquares> generate(const std::vector<std::string>& inputs) noexcept;

    std::vector<std::string> toString(const Solution& s) noexcept override;

    virtual ~LatinSquares() noexcept = default;

protected:
    LatinSquares(const std::vector<bool>&        data,
                 size_t                          rows,
                 size_t                          cols,
                 const std::vector<int>&         rowsList,
                 const std::vector<std::string>& inputs) noexcept;

private:
    const std::vector<std::string> _initState;
};

} // namespace ecv

#endif // INCLUDE_ECV_HPP
