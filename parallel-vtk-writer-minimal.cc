#include <sstream>
#include <vector>

#include <omp.h>

#include <vtkCPDataDescription.h>
#include <vtkDoubleArray.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkMultiPieceDataSet.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPMultiBlockDataWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

void WriteUnstructuredGrids(int max_threads,
                            std::vector<vtkUnstructuredGrid*> grids) {
  std::cout << "WriteUnstructuredGrids" << std::endl;

#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < max_threads; ++i) {
    vtkNew<vtkXMLUnstructuredGridWriter> vtu_writer;
    std::stringstream s;
    s << "Cells_" << i << ".vtu";
    vtu_writer->SetFileName(s.str().c_str());
    vtu_writer->SetInputData(grids[i]);
    vtu_writer->Write();
  }

  vtkNew<vtkXMLPUnstructuredGridWriter> pvtu_writer;
  pvtu_writer->SetFileName("Cells.pvtu");
  pvtu_writer->SetNumberOfPieces(max_threads);
  // ERROR: Attempt to connect input port index 1 for an algorithm with 1 input ports. 
  // for(int i = 0; i < max_threads; ++i) {
  //   pvtu_writer->SetInputData(i, grids[i]);
  // }
  pvtu_writer->SetInputData(grids[0]);
  pvtu_writer->Write();
}

void WriteMultiPieceDataSet(int max_threads,
                            std::vector<vtkUnstructuredGrid*> grids) {
  std::cout << "WriteMultiPieceDataSet" << std::endl;

  vtkNew<vtkMultiPieceDataSet> mp;
  mp->SetNumberOfPieces(max_threads);
  for (int i = 0; i < max_threads; ++i) {
    mp->SetPiece(i, grids[i]);
  }

  vtkNew<vtkXMLPUnstructuredGridWriter> pvtu_writer;
  pvtu_writer->SetFileName("multi-piece-data.pvtu");
  pvtu_writer->SetInputData(mp.GetPointer());
  pvtu_writer->SetNumberOfPieces(max_threads);
  pvtu_writer->Write();
}

void WriteMultiBlockDataSet(int max_threads,
                            std::vector<vtkUnstructuredGrid*> grids) {
  std::cout << "WriteMultiBlockDataSet" << std::endl;

  vtkNew<vtkMultiPieceDataSet> mp;
  mp->SetNumberOfPieces(max_threads);
  for (int i = 0; i < max_threads; ++i) {
    mp->SetPiece(i, grids[i]);
  }

  vtkNew<vtkMultiBlockDataSet> mb;
  mb->SetNumberOfBlocks(1);
  mb->SetBlock(0, mp.GetPointer());

  vtkNew<vtkXMLPMultiBlockDataWriter> pwriter;
  pwriter->SetFileName("multi-block-data");
  pwriter->SetInputData(mb.GetPointer());
  pwriter->SetNumberOfPieces(max_threads);
  pwriter->Write();
}

int main(int argc, const char** argv) {
  int max_threads = omp_get_max_threads();

  // construct
  std::vector<vtkUnstructuredGrid*> grids;
  grids.resize(max_threads);
  for (int i = 0; i < max_threads; ++i) {
    grids[i] = vtkUnstructuredGrid::New();
  }

  // add data arrays
  for (int i = 0; i < max_threads; ++i) {
    // position array
    {
      vtkNew<vtkDoubleArray> new_vtk_array;
      new_vtk_array->SetName("position");
      auto* vtk_array = new_vtk_array.GetPointer();
      vtk_array->SetNumberOfComponents(3);
      vtk_array->InsertNextTuple3(0, 0, 10 * i);
      vtkNew<vtkPoints> points;
      points->SetData(vtk_array);
      grids[i]->SetPoints(points.GetPointer());
    }

    // diameter array
    {
      vtkNew<vtkDoubleArray> new_vtk_array;
      new_vtk_array->SetName("diameter");
      auto* vtk_array = new_vtk_array.GetPointer();
      vtk_array->SetNumberOfComponents(1);
      vtk_array->InsertNextTuple1(5 * ((i % 5) + 1));
      auto* point_data = grids[i]->GetPointData();
      point_data->AddArray(vtk_array);
    }
  }

  // write
  if (argc != 2) {
    WriteUnstructuredGrids(max_threads, grids);
  } else {
    switch (argv[1][0]) {
      case 'P':
        WriteMultiPieceDataSet(max_threads, grids);
        break;
      case 'B':
        WriteMultiBlockDataSet(max_threads, grids);
        break;
      case 'U':
      default:
        WriteUnstructuredGrids(max_threads, grids);
        break;
    }
  }

  // destruct
  for (int i = 0; i < max_threads; ++i) {
    grids[i]->Delete();
  }

  return 0;
}
