# ECV

**:star2: C++ library implementing DLX algorithm :star2:**

Just another C++ implementation of [Donald Knuth's Algorithm X](https://arxiv.org/pdf/cs/0011047v1.pdf) for exact cover problem.

## How to build/install

The **ecv** library consists of a public header [**include/ecv.hpp**](./include/ecv.hpp) which contains high-level structures to create and solve exact cover problems :

**Note** : Everything lives under the **namespace ecv**.

Solve a problem using DLX :
- **DLX** is the DLX implementation. Concrete exact cover problems inherit from it.
 - **DLX::solve(uint32_t max_nb)** solves the problem and generate at most **max_nb** solutions to the problem.
 - **DLX::apply(const Solution&)** returns the problem state when applying one of its solutions.

Create a solvable concrete problem :
 - **LatinSquares::generate()** generates a concrete "Latin square" problem.

Here is an example : 

```
#include <ecv.hpp>

#include <iostream>

using namespace ecv;

int main() {
	// Initial state
	State myProblem{ "00000", "00000", "00000", "00000", "00000" };

	// Generate the Latin square problem from our initial state
	auto obj{ LatinSquares::generate(myProblem) };

	if ( nullptr == obj ) 
		return EXIT_FAILURE;

	// Get every solutions to the problem
	auto solutions{ obj->solve() };
	if ( std::empty(solutions) ) {
		std::cout << "no solutions\n";
		return EXIT_SUCCESS;
	}

	// Get the final state when applying one of our solutions
	State myProblemSolved{ obj->apply(solutions[0]) };
	
	// Let's print it 
	for     ( const auto& line : myProblemSolved ) {
		for   ( const auto& cell : line            )
				std::cout << cell << " ";
		std::cout << '\n';
	}

	return EXIT_SUCCESS;
}
```

```
[~/tests/ecv] ./test-ecv 
5 1 2 3 4 
1 4 5 2 3 
2 5 3 4 1 
3 2 4 1 5 
4 3 1 5 2
```

### Build

```
[~/builds/ecv] cmake -S ~${YOUR_ECV_PATH} -DCMAKE_INSTALL_PREFIX=${YOUR_INSTALL_DIR}
-- Configuring done
-- Generating done
-- Build files have been written to: ${YOUR_INSTALL_DIR}/ecv
```

### Install

```
[~/builds/ecv] make install
[100%] Built target ecv
Install the project...
-- Install configuration: ""
-- Installing: ${YOUR_INSTALL_DIR}/ecv/lib/libecv.a
-- Installing: ${YOUR_INSTALL_DIR}/ecv/include/ecv.hpp
```

## TODO

- [ ] Add common [exact cover problems](https://en.wikipedia.org/wiki/Exact_cover) implementations
  - [x] Latin square
  - [ ] Sudoku
  - [ ] N Queens problem
  - [ ] Pentomino tiling
  - ...
- [ ] Better error handling
- [ ] Add built-in examples
- [ ] Add documentation