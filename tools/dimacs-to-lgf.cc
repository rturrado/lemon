/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2003-2009
 * Egervary Jeno Kombinatorikus Optimalizalasi Kutatocsoport
 * (Egervary Research Group on Combinatorial Optimization, EGRES).
 *
 * Permission to use, modify and distribute this software is granted
 * provided that this copyright notice appears in all copies. For
 * precise terms see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any kind,
 * express or implied, and with no claim as to its suitability for any
 * purpose.
 *
 */

///\ingroup tools
///\file
///\brief DIMACS to LGF converter.
///
/// This program converts various DIMACS formats to the LEMON Digraph Format
/// (LGF).
///
/// See
/// \code
///   dimacs-to-lgf --help
/// \endcode
/// for more info on the usage.

#include <iostream>
#include <fstream>
#include <cstring>

#include <lemon/smart_graph.h>
#include <lemon/dimacs.h>
#include <lemon/lgf_writer.h>

#include <lemon/arg_parser.h>
#include <lemon/error.h>

using namespace std;
using namespace lemon;


int main(int argc, const char *argv[]) {
  typedef SmartDigraph Digraph;

  typedef Digraph::Arc Arc;
  typedef Digraph::Node Node;
  typedef Digraph::ArcIt ArcIt;
  typedef Digraph::NodeIt NodeIt;
  typedef Digraph::ArcMap<double> DoubleArcMap;
  typedef Digraph::NodeMap<double> DoubleNodeMap;

  std::string inputName;
  std::string outputName;

  ArgParser ap(argc, argv);
  ap.other("[INFILE [OUTFILE]]",
           "If either the INFILE or OUTFILE file is missing the standard\n"
           "     input/output will be used instead.")
    .run();

  ifstream input;
  ofstream output;

  const auto& files = ap.files();
  const auto files_size = files.size();
  if (files_size > 2){
    std::cerr << ap.commandName() << ": too many arguments\n";
    return 1;
  } else {
    if (files_size > 0) {
      input.open(files[0].c_str());
      if (!input) {
        throw IoError("File cannot be found: ", files[0]);
      }
    }
    if (files_size > 1) {
      output.open(files[1].c_str());
      if (!output) {
        throw IoError("Cannot open the file for writing: ", files[1]);
      }
    }
  }
  std::istream& is = ((files_size < 1) ? std::cin : input);
  std::ostream& os = ((files_size < 2) ? std::cout : output);

  DimacsDescriptor desc = dimacsType(is);
  switch(desc.type)
    {
    case DimacsDescriptor::MIN:
      {
        Digraph digraph;
        DoubleArcMap lower(digraph), capacity(digraph), cost(digraph);
        DoubleNodeMap supply(digraph);
        readDimacsMin(is, digraph, lower, capacity, cost, supply, 0, desc);
        DigraphWriter<Digraph>(digraph, os).
          nodeMap("supply", supply).
          arcMap("lower", lower).
          arcMap("capacity", capacity).
          arcMap("cost", cost).
          attribute("problem","min").
          run();
      }
      break;
    case DimacsDescriptor::MAX:
      {
        Digraph digraph;
        Node s, t;
        DoubleArcMap capacity(digraph);
        readDimacsMax(is, digraph, capacity, s, t, 0, desc);
        DigraphWriter<Digraph>(digraph, os).
          arcMap("capacity", capacity).
          node("source", s).
          node("target", t).
          attribute("problem","max").
          run();
      }
      break;
    case DimacsDescriptor::SP:
      {
        Digraph digraph;
        Node s;
        DoubleArcMap capacity(digraph);
        readDimacsSp(is, digraph, capacity, s, desc);
        DigraphWriter<Digraph>(digraph, os).
          arcMap("capacity", capacity).
          node("source", s).
          attribute("problem","sp").
          run();
      }
      break;
    case DimacsDescriptor::MAT:
      {
        Digraph digraph;
        readDimacsMat(is, digraph,desc);
        DigraphWriter<Digraph>(digraph, os).
          attribute("problem","mat").
          run();
      }
      break;
    default:
      break;
    }
  return 0;
}
