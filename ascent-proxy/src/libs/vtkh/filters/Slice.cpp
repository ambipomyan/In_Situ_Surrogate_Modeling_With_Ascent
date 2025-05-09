#include <vtkh/filters/Slice.hpp>
#include <vtkh/Error.hpp>
#include <vtkh/filters/MarchingCubes.hpp>
#include <vtkh/filters/CleanGrid.hpp>
#include <vtkh/filters/IsoVolume.hpp>
#include <vtkh/vtkm_filters/vtkmClip.hpp>
#include <vtkm/filter/contour/Slice.h>

#include <vtkm/VectorAnalysis.h>
#include <vtkm/cont/Algorithm.h>
#include <vtkm/cont/TryExecute.h>
#include <vtkm/ImplicitFunction.h>
#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>

namespace vtkh
{

namespace detail
{

struct print_f
{
  template<typename T, typename S>
  void operator()(const vtkm::cont::ArrayHandle<T,S> &a) const
  {
    vtkm::Id s = a.GetNumberOfValues();
    auto p = a.ReadPortal();
    for(int i = 0; i < s; ++i)
    {
      std::cout<<p.Get(i)<<" ";
    }
    std::cout<<"\n";
  }
};

class SliceField : public vtkm::worklet::WorkletMapField
{
protected:
  vtkm::Vec<vtkm::Float32,3> m_point;
  vtkm::Vec<vtkm::Float32,3> m_normal;
public:
  VTKM_CONT
  SliceField(vtkm::Vec<vtkm::Float32,3> point, vtkm::Vec<vtkm::Float32,3> normal)
    : m_point(point),
      m_normal(normal)
  {
    vtkm::Normalize(m_normal);
  }

  typedef void ControlSignature(FieldIn, FieldOut);
  typedef void ExecutionSignature(_1, _2);

  template<typename T>
  VTKM_EXEC
  void operator()(const vtkm::Vec<T,3> &point, vtkm::Float32& distance) const
  {
    vtkm::Vec<vtkm::Float32,3> f_point(point[0], point[1], point[2]);
    distance = vtkm::dot(m_point - f_point, m_normal);
  }
}; //class SliceField

class Offset : public vtkm::worklet::WorkletMapField
{
protected:
  vtkm::Id m_offset;

public:
  VTKM_CONT
  Offset(const vtkm::Id offset)
    : m_offset(offset)
  {
  }

  typedef void ControlSignature(FieldIn, WholeArrayInOut);
  typedef void ExecutionSignature(_1, _2);

  template<typename PortalType>
  VTKM_EXEC
  void operator()(const vtkm::Id &index, PortalType values) const
  {
    vtkm::Id value = values.Get(index);
    values.Set(index, value + m_offset);
  }
}; //class Offset

//---------------------------------------------------------------------------//
// Multi Plane Implicit Function 
//---------------------------------------------------------------------------//
class MultiPlane : public vtkm::internal::ImplicitFunctionBase<MultiPlane>
{
public:
  MultiPlane() = default;

  //-------------------------------------------------------------------------//
  VTKM_EXEC_CONT MultiPlane(const Vector points[3],
                            const Vector normals[3],
                            const int num_planes)
  {
    this->SetPlanes(points, normals);
    this->m_num_planes = num_planes;
  }

  //-------------------------------------------------------------------------//
  VTKM_EXEC void SetPlanes(const Vector points[6], const Vector normals[6])
  {
    for (vtkm::Id index : { 0, 1, 2})
    {
      this->Points[index] = points[index];
    }
    for (vtkm::Id index : { 0, 1, 2})
    {
      this->Normals[index] = normals[index];
    }
  }

  //-------------------------------------------------------------------------//
  VTKM_EXEC void SetPlane(int idx, const Vector& point, const Vector& normal)
  {
    VTKM_ASSERT((idx >= 0) && (idx < 3));
    this->Points[idx] = point;
    this->Normals[idx] = normal;
  }

  //-------------------------------------------------------------------------//
  VTKM_EXEC_CONT void SetNumPlanes(const int &num)
  {
    this->m_num_planes = num;
  }

  //-------------------------------------------------------------------------//
  VTKM_EXEC_CONT void GetPlanes(Vector points[3], Vector normals[3]) const
  {
    for (vtkm::Id index : { 0, 1, 2})
    {
      points[index] = this->Points[index];
    }
    for (vtkm::Id index : { 0, 1, 2})
    {
      normals[index] = this->Normals[index];
    }
  }

  //-------------------------------------------------------------------------//
  VTKM_EXEC_CONT const Vector* GetPoints() const { return this->Points; }

  //-------------------------------------------------------------------------//
  VTKM_EXEC_CONT const Vector* GetNormals() const { return this->Normals; }

  //-------------------------------------------------------------------------//
  VTKM_EXEC_CONT Scalar Value(const Vector& point) const
  {
    Scalar maxVal = vtkm::NegativeInfinity<Scalar>();
    for (vtkm::Id index = 0; index < this->m_num_planes; ++index)
    {
      const Vector& p = this->Points[index];
      const Vector& n = this->Normals[index];
      const Scalar val = vtkm::Dot(point - p, n);
      maxVal = vtkm::Max(maxVal, val);
    }
    return maxVal;
  }

  //-------------------------------------------------------------------------//
  VTKM_EXEC_CONT Vector Gradient(const Vector& point) const
  {
    Scalar maxVal = vtkm::NegativeInfinity<Scalar>();
    vtkm::Id maxValIdx = 0;
    for (vtkm::Id index = 0; index < this->m_num_planes; ++index)
    {
      const Vector& p = this->Points[index];
      const Vector& n = this->Normals[index];
      Scalar val = vtkm::Dot(point - p, n);
      if (val > maxVal)
      {
        maxVal = val;
        maxValIdx = index;
      }
    }
    return this->Normals[maxValIdx];
  }

private:
  Vector Points[6] = { { -0.0f, 0.0f, 0.0f },
                       { 0.0f, 0.0f, 0.0f },
                       { 0.0f, -0.0f, 0.0f }};
  Vector Normals[6] = { { -1.0f, 0.0f, 0.0f },
                        { 1.0f, 0.0f, 0.0f },
                        { 0.0f, 0.0f, 0.0f } };
  int m_num_planes = 3;
}; // MultiPlane

//---------------------------------------------------------------------------//
/// Multi Plane Field 
//---------------------------------------------------------------------------//
class MultiPlaneField : public vtkm::worklet::WorkletMapField
{
protected:
  MultiPlane m_multi_plane;
public:
  //-------------------------------------------------------------------------//
  VTKM_CONT MultiPlaneField(MultiPlane &multi_plane)
    : m_multi_plane(multi_plane)
  {
  }

  typedef void ControlSignature(FieldIn, FieldOut);
  typedef void ExecutionSignature(_1, _2);

  //-------------------------------------------------------------------------//
  template<typename T>
  VTKM_EXEC
  void operator()(const vtkm::Vec<T,3> &point, vtkm::Float32& distance) const
  {
    distance = m_multi_plane.Value(point);
  }
}; //class MultiPlaneField

class MergeContours
{
  std::vector<vtkh::DataSet*> &m_data_sets;
  std::string m_skip_field; // we skip the slice field
public:
  MergeContours(std::vector<vtkh::DataSet*> &data_sets, std::string skip_field)
    : m_data_sets(data_sets),
      m_skip_field(skip_field)
  {}

  ~MergeContours()
  {
    for(size_t i = 0; i < m_data_sets.size(); ++i)
    {
      delete m_data_sets[i];
    }
  }

  std::vector<vtkm::Id> UnionDomainIds()
  {
    std::vector<vtkm::Id> domain_ids;
    const size_t num_dsets = m_data_sets.size();
    for(size_t i = 0; i < num_dsets; ++i)
    {
      std::vector<vtkm::Id> add = m_data_sets[i]->GetDomainIds();
      domain_ids.insert(domain_ids.end(), add.begin(), add.end());
    }

    std::sort(domain_ids.begin(), domain_ids.end());
    auto last = std::unique(domain_ids.begin(), domain_ids.end());
    domain_ids.erase(last, domain_ids.end());
    return domain_ids;
  }

  template<typename U>
  struct CopyFunctor
  {
    vtkm::cont::ArrayHandle<vtkm::Vec<U,3>> output;
    vtkm::Id offset;

    template<typename Type, typename S>
    void operator()(vtkm::cont::ArrayHandle<Type,S> &input)
    {
      vtkm::Id copy_size = input.GetNumberOfValues();
      vtkm::Id start = 0;
      vtkm::cont::Algorithm::CopySubRange(input, start, copy_size, output, offset);
    }
  };
  template<typename T, typename S, typename U>
  void CopyCoords(vtkm::cont::UncertainArrayHandle<T,S> &input,
                  vtkm::cont::ArrayHandle<vtkm::Vec<U,3>> &output,
                  vtkm::Id offset)
  {
    CopyFunctor<U> func{output,offset};
    input.CastAndCall(func);
  }

  struct CopyField
  {
    vtkm::cont::DataSet &m_data_set;
    std::vector<vtkm::cont::DataSet> m_in_data_sets;
    vtkm::Id *m_point_offsets;
    vtkm::Id *m_cell_offsets;
    vtkm::Id  m_field_index;
    vtkm::Id  m_num_points;
    vtkm::Id  m_num_cells;

    CopyField(vtkm::cont::DataSet &data_set,
              std::vector<vtkm::cont::DataSet> in_data_sets,
              vtkm::Id *point_offsets,
              vtkm::Id *cell_offsets,
              vtkm::Id num_points,
              vtkm::Id num_cells,
              vtkm::Id field_index)
      : m_data_set(data_set),
        m_in_data_sets(in_data_sets),
        m_point_offsets(point_offsets),
        m_cell_offsets(cell_offsets),
        m_field_index(field_index),
        m_num_points(num_points),
        m_num_cells(num_cells)
    {}

    template<typename T, typename S>
    void operator()(const vtkm::cont::ArrayHandle<T,S> &vtkmNotUsed(field)) const
    {
      //check to see if this is a supported field ;
      const vtkm::cont::Field &scalar_field = m_in_data_sets[0].GetField(m_field_index);
      bool is_supported = (scalar_field.GetAssociation() == vtkm::cont::Field::Association::Points ||
                           scalar_field.GetAssociation() == vtkm::cont::Field::Association::Cells);

      if(!is_supported) return;

      bool assoc_points = scalar_field.GetAssociation() == vtkm::cont::Field::Association::Points;
      vtkm::cont::ArrayHandle<T> out;
      if(assoc_points)
      {
        out.Allocate(m_num_points);
      }
      else
      {
        out.Allocate(m_num_cells);
      }

      for(size_t i = 0; i < m_in_data_sets.size(); ++i)
      {
        const vtkm::cont::Field &f = m_in_data_sets[i].GetField(m_field_index);
        vtkm::cont::ArrayHandle<T,S> in = f.GetData().AsArrayHandle<vtkm::cont::ArrayHandle<T,S>>();
        vtkm::Id start = 0;
        vtkm::Id copy_size = in.GetNumberOfValues();
        vtkm::Id offset = assoc_points ? m_point_offsets[i] : m_cell_offsets[i];

        vtkm::cont::Algorithm::CopySubRange(in, start, copy_size, out, offset);
      }

      vtkm::cont::Field out_field(scalar_field.GetName(),
                                  scalar_field.GetAssociation(),
                                  out);
      m_data_set.AddField(out_field);

    }
  };

  vtkm::cont::DataSet MergeDomains(std::vector<vtkm::cont::DataSet> &doms)
  {
    vtkm::cont::DataSet res;

    vtkm::Id num_cells = 0;
    vtkm::Id num_points = 0;
    std::vector<vtkm::Id> cell_offsets(doms.size());
    std::vector<vtkm::Id> point_offsets(doms.size());

    for(size_t dom = 0; dom < doms.size(); ++dom)
    {
      auto cell_set = doms[dom].GetCellSet();

      // In the past, we were making assumptions that the output of contour
      // was a cell set single type. Because of difficult vtkm reasons, the output
      // of contour is now explicit cell set,but we can still assume that
      // this output will be all triangles.
      // this becomes more complicated if we want to support mixed types
      //if(!cell_set.IsType(vtkm::cont::CellSetSingleType<>())) continue;
      if(!cell_set.IsType<vtkm::cont::CellSetExplicit<>>())
      {
        std::cout<<"expected explicit cell set as the result of contour\n";

        continue;
      }

      cell_offsets[dom] = num_cells;
      num_cells += cell_set.GetNumberOfCells();

      auto coords = doms[dom].GetCoordinateSystem();
      point_offsets[dom] = num_points;
      num_points += coords.GetData().GetNumberOfValues();

    }

    const vtkm::Id conn_size = num_cells * 3;

    // calculate merged offsets for all domains
    vtkm::cont::ArrayHandle<vtkm::Id> conn;
    conn.Allocate(conn_size);

    // handle coordinate merging
    vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float64, 3>> out_coords;
    out_coords.Allocate(num_points);
    // coordinate type that contour produces
    //using CoordsType3f = vtkm::cont::ArrayHandleVirtual<vtkm::Vec<vtkm::Float32,3>>;
    //using CoordsType3d = vtkm::cont::ArrayHandleVirtual<vtkm::Vec<vtkm::Float64,3>>;

    for(size_t dom = 0; dom < doms.size(); ++dom)
    {
      auto cell_set = doms[dom].GetCellSet();

      //if(!cell_set.IsType(vtkm::cont::CellSetSingleType<>())) continue;
      if(!cell_set.IsType<vtkm::cont::CellSetExplicit<>>())
      {
        std::cout<<"expected explicit cell set as the result of contour\n";
        continue;
      }

      // grab the connectivity and copy it into the larger array
      //vtkm::cont::CellSetSingleType<> single_type = cell_set.Cast<vtkm::cont::CellSetSingleType<>>();
      vtkm::cont::CellSetExplicit<> single_type =
        cell_set.AsCellSet<vtkm::cont::CellSetExplicit<>>();
      const vtkm::cont::ArrayHandle<vtkm::Id> dconn = single_type.GetConnectivityArray(
        vtkm::TopologyElementTagCell(),
        vtkm::TopologyElementTagPoint());

      vtkm::Id copy_size = dconn.GetNumberOfValues();
      vtkm::Id start = 0;

      vtkm::cont::Algorithm::CopySubRange(dconn, start, copy_size, conn, cell_offsets[dom]*3);
      // now we offset the connectiviy we just copied in so we references the
      // correct points
      if(cell_offsets[dom] != 0)
      {
        vtkm::cont::ArrayHandleCounting<vtkm::Id> indexes(cell_offsets[dom]*3, 1, copy_size);
        vtkm::worklet::DispatcherMapField<detail::Offset>(detail::Offset(point_offsets[dom]))
          .Invoke(indexes, conn);
      }

      // merge coodinates
      auto coords = doms[dom].GetCoordinateSystem().GetData();
      this->CopyCoords(coords, out_coords, point_offsets[dom]);
      //if(coords == CoordsType3f())
      //{
      //  CoordsType3f in = coords.Cast<CoordsType3f>();
      //  this->CopyCoords(in, out_coords, point_offsets[dom]);
      //}
      //if(coords.IsType<CoordsType3d>())
      //{
      //  CoordsType3d in = coords.Cast<CoordsType3d>();
      //  this->CopyCoords(in, out_coords, point_offsets[dom]);
      //}
      //else
      //{
      //  throw Error("Merge contour: unknown coordinate type");
      //}

    } // for each domain


    vtkm::cont::CellSetSingleType<> cellSet;
    cellSet.Fill(num_points, vtkm::CELL_SHAPE_TRIANGLE, 3, conn);
    res.SetCellSet(cellSet);

    res.AddCoordinateSystem(vtkm::cont::CoordinateSystem("coords", out_coords));

    // handle fields, they are all the same since they came from the same data set
    const int num_fields = doms[0].GetNumberOfFields();

    for(int f = 0; f < num_fields; ++f)
    {
      const vtkm::cont::Field &field = doms[0].GetField(f);

      if(field.GetName() == m_skip_field) continue;

      CopyField copier(res,
                       doms,
                       &point_offsets[0],
                       &cell_offsets[0],
                       num_points,
                       num_cells,
                       f);

      auto full = field.GetData().ResetTypes(vtkm::TypeListCommon(),VTKM_DEFAULT_STORAGE_LIST{});
      full.CastAndCall(copier);
    }
    return res;
  }

  vtkh::DataSet* Merge()
  {
    std::vector<vtkm::Id> domain_ids = this->UnionDomainIds();
    vtkh::DataSet *res = new vtkh::DataSet();
    for(size_t dom = 0; dom < domain_ids.size(); ++dom)
    {
      // gather domain
      std::vector<vtkm::cont::DataSet> doms;
      vtkm::Id domain_id = domain_ids[dom];
      for(size_t i = 0; i < m_data_sets.size(); ++i)
      {
        if(m_data_sets[i]->HasDomainId(domain_id))
        {
          doms.push_back(m_data_sets[i]->GetDomainById(domain_id));
        }

      } // for each data set
      res->AddDomain(this->MergeDomains(doms), domain_id);
    } // for each domain id

    return res;
  }

};

template <typename T>
std::vector<T>
GetScalarData(vtkh::DataSet &vtkhData, const char *field_name)
{
  //Get domain Ids on this rank
  //will be nonzero even if there is no data
  std::vector<vtkm::Id> localDomainIds = vtkhData.GetDomainIds();
  std::vector<T> data;
     
  //if there is data: loop through domains and grab all triangles.
  if(!vtkhData.IsEmpty())
  {
    for(int i = 0; i < localDomainIds.size(); i++)
    {
      vtkm::cont::DataSet dataset = vtkhData.GetDomainById(localDomainIds[i]);
      vtkm::cont::CoordinateSystem coords = dataset.GetCoordinateSystem();
      vtkm::cont::UnknownCellSet cellset = dataset.GetCellSet();
      //Get variable
      vtkm::cont::Field field = dataset.GetField(field_name);
      
      long int size = field.GetNumberOfValues();
      
      using data_d = vtkm::cont::ArrayHandle<vtkm::Float64>;
      using data_f = vtkm::cont::ArrayHandle<vtkm::Float32>;
      if(field.GetData().IsType<data_d>())
      {
        vtkm::cont::ArrayHandle<vtkm::Float64> field_data;
        field.GetData().AsArrayHandle(field_data);
        auto portal = field_data.ReadPortal();

        for(int i = 0; i < size; i++)
        {
          data.push_back(portal.Get(i));
        }
      }
      if(field.GetData().IsType<data_f>())
      {
        vtkm::cont::ArrayHandle<vtkm::Float32> field_data;
        field.GetData().AsArrayHandle(field_data);
        auto portal = field_data.ReadPortal();

        for(int i = 0; i < size; i++)
        {
          data.push_back(portal.Get(i));
        }
      }
    }
  }
  //else
    //cerr << "VTKH Data is empty" << endl;
  return data;
}

template< typename T >
T calcEntropyMM( const std::vector<T> array, long len, int nBins , T field_min, T field_max)
{
  T min = field_min;
  T max = field_max;

  T stepSize = (max-min) / (T)nBins;
  if(stepSize == 0)
    return 0.0;

  long* hist = new long[ nBins ];
  for(int i = 0; i < nBins; i++ )
    hist[i] = 0;

  for(long i = 0; i < len; i++ )
  {
    T idx = (std::abs(array[i]) - min) / stepSize;
    if((int)idx == nBins )
      idx -= 1.0;
    hist[(int)idx]++;
  }

  T entropy = 0.0;
  for(int i = 0; i < nBins; i++ )
  {
    T prob = (T)hist[i] / (T)len;
    if(prob != 0.0 )
      entropy += prob * std::log( prob );
  }

  delete[] hist;

  return (entropy * -1.0);
}


} // namespace detail

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
// Slice
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
Slice::Slice()
{

}

//---------------------------------------------------------------------------//
Slice::~Slice()
{

}

//---------------------------------------------------------------------------//
void
Slice::AddPlane(vtkm::Vec<vtkm::Float32,3> point,
                vtkm::Vec<vtkm::Float32,3> normal)
{
  m_points.push_back(point);
  m_normals.push_back(normal);
}

//---------------------------------------------------------------------------//
void
Slice::PreExecute()
{
  Filter::PreExecute();
}

//---------------------------------------------------------------------------//
void
Slice::DoExecute()
{
  const std::string fname = "slice_field";
  const int num_domains = this->m_input->GetNumberOfDomains();
  const int num_slices = this->m_points.size();

  if(num_slices == 0)
  {
    throw Error("Slice: no slice planes specified");
  }

  std::vector<vtkh::DataSet*> slices;
  for(int s = 0; s < num_slices; ++s)
  {
    vtkm::Vec<vtkm::Float32,3> point = m_points[s];
    vtkm::Vec<vtkm::Float32,3> normal = m_normals[s];
    vtkh::DataSet temp_ds = *(this->m_input);
    // shallow copy the input so we don't propagate the slice field
    // to the input data set, since it might be used in other places
    for(int i = 0; i < num_domains; ++i)
    {
      vtkm::cont::DataSet &dom = temp_ds.GetDomain(i);

      vtkm::cont::ArrayHandle<vtkm::Float32> slice_field;
      vtkm::worklet::DispatcherMapField<detail::SliceField>(detail::SliceField(point, normal))
        .Invoke(dom.GetCoordinateSystem().GetData(), slice_field);

      dom.AddField(vtkm::cont::Field(fname,
                                      vtkm::cont::Field::Association::Points,
                                      slice_field));
    } // each domain

    vtkh::MarchingCubes marcher;
    marcher.SetInput(&temp_ds);
    marcher.SetIsoValue(0.);
    marcher.SetField(fname);
    marcher.Update();
    slices.push_back(marcher.GetOutput());
  } // each slice

  if(slices.size() > 1)
  {
    detail::MergeContours merger(slices, fname);
    this->m_output = merger.Merge();
  }
  else
  {
    this->m_output = slices[0];
  }
}

//---------------------------------------------------------------------------//
void
Slice::PostExecute()
{
  Filter::PostExecute();
}

//---------------------------------------------------------------------------//
std::string
Slice::GetName() const
{
  return "vtkh::Slice";
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
// Slice Implicit
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
struct SliceImplicit::InternalsType
{
  vtkm::ImplicitFunctionGeneral m_func;
  detail::MultiPlane m_multi_plane;
  InternalsType()
  {}
};


//---------------------------------------------------------------------------//
// Slice Implicit VTK-h filter
//---------------------------------------------------------------------------//
SliceImplicit::SliceImplicit()
  : m_internals(new InternalsType),
    m_do_multi_plane(false)
{

}

//---------------------------------------------------------------------------//
SliceImplicit::~SliceImplicit()
{

}

//---------------------------------------------------------------------------//
void
SliceImplicit::SetBoxSlice(const vtkm::Bounds &clipping_bounds)
{
  m_do_multi_plane = false;
  auto box = vtkm::Box({ clipping_bounds.X.Min,
                         clipping_bounds.Y.Min,
                         clipping_bounds.Z.Min},
                       { clipping_bounds.X.Max,
                         clipping_bounds.Y.Max,
                         clipping_bounds.Z.Max});

  m_internals->m_func = box;
}

//---------------------------------------------------------------------------//
void
SliceImplicit::SetSphereSlice(const double center[3], const double radius)
{
  m_do_multi_plane = false;
  vtkm::Vec<vtkm::FloatDefault,3> vec_center;
  vec_center[0] = center[0];
  vec_center[1] = center[1];
  vec_center[2] = center[2];
  vtkm::FloatDefault r = radius;

  auto sphere = vtkm::Sphere(vec_center, r);
  m_internals->m_func = sphere;
}

//---------------------------------------------------------------------------//
void
SliceImplicit::SetCylinderSlice(const double center[3],
                                const double axis[3],
                                const double radius)
{
  m_do_multi_plane = false;
  vtkm::Vec<vtkm::FloatDefault,3> vec_center;
  vec_center[0] = center[0];
  vec_center[1] = center[1];
  vec_center[2] = center[2];

  vtkm::Vec<vtkm::FloatDefault,3> vec_axis;
  vec_axis[0] = axis[0];
  vec_axis[1] = axis[1];
  vec_axis[2] = axis[2];

  vtkm::FloatDefault r = radius;

  auto cylinder = vtkm::Cylinder(vec_center, vec_axis, r);
  m_internals->m_func = cylinder;
}

//---------------------------------------------------------------------------//
void
SliceImplicit::SetPlaneSlice(const double origin[3],
                             const double normal[3])
{
  m_do_multi_plane = false;
  vtkm::Vec<vtkm::FloatDefault,3> vec_origin;
  vec_origin[0] = origin[0];
  vec_origin[1] = origin[1];
  vec_origin[2] = origin[2];

  vtkm::Vec<vtkm::FloatDefault,3> vec_normal;
  vec_normal[0] = normal[0];
  vec_normal[1] = normal[1];
  vec_normal[2] = normal[2];

  auto plane = vtkm::Plane(vec_origin, vec_normal);
  m_internals->m_func = plane;
}

//
// TODO: Multi Plane Implicit needs work.
//
// //---------------------------------------------------------------------------//
// void
// SliceImplicit::Set2PlaneSlice(const double origin1[3],
//                               const double normal1[3],
//                               const double origin2[3],
//                               const double normal2[3])
// {
//   m_do_multi_plane = true;
//   vtkm::Vec3f plane_points[3];
//   plane_points[0][0] = float(origin1[0]);
//   plane_points[0][1] = float(origin1[1]);
//   plane_points[0][2] = float(origin1[2]);
//
//   plane_points[1][0] = float(origin2[0]);
//   plane_points[1][1] = float(origin2[1]);
//   plane_points[1][2] = float(origin2[2]);
//
//   plane_points[2][0] = 0.f;
//   plane_points[2][1] = 0.f;
//   plane_points[2][2] = 0.f;
//
//   vtkm::Vec3f plane_normals[3];
//   plane_normals[0][0] = float(normal1[0]);
//   plane_normals[0][1] = float(normal1[1]);
//   plane_normals[0][2] = float(normal1[2]);
//
//   plane_normals[1][0] = float(normal2[0]);
//   plane_normals[1][1] = float(normal2[1]);
//   plane_normals[1][2] = float(normal2[2]);
//
//   plane_normals[2][0] = 0.f;
//   plane_normals[2][1] = 0.f;
//   plane_normals[2][2] = 0.f;
//
//   vtkm::Normalize(plane_normals[0]);
//   vtkm::Normalize(plane_normals[1]);
//
//   auto planes
//     = detail::MultiPlane(plane_points, plane_normals, 2);
//   m_internals->m_multi_plane = planes;
// }
//
// //---------------------------------------------------------------------------//
// void
// SliceImplicit::Set3PlaneSlice(const double origin1[3],
//                               const double normal1[3],
//                               const double origin2[3],
//                               const double normal2[3],
//                               const double origin3[3],
//                               const double normal3[3])
// {
//   m_do_multi_plane = true;
//   vtkm::Vec3f plane_points[3];
//   plane_points[0][0] = float(origin1[0]);
//   plane_points[0][1] = float(origin1[1]);
//   plane_points[0][2] = float(origin1[2]);
//
//   plane_points[1][0] = float(origin2[0]);
//   plane_points[1][1] = float(origin2[1]);
//   plane_points[1][2] = float(origin2[2]);
//
//   plane_points[2][0] = float(origin3[0]);
//   plane_points[2][1] = float(origin3[1]);
//   plane_points[2][2] = float(origin3[2]);
//
//   vtkm::Vec3f plane_normals[3];
//   plane_normals[0][0] = float(normal1[0]);
//   plane_normals[0][1] = float(normal1[1]);
//   plane_normals[0][2] = float(normal1[2]);
//
//   plane_normals[1][0] = float(normal2[0]);
//   plane_normals[1][1] = float(normal2[1]);
//   plane_normals[1][2] = float(normal2[2]);
//
//   plane_normals[2][0] = float(normal3[0]);
//   plane_normals[2][1] = float(normal3[1]);
//   plane_normals[2][2] = float(normal3[2]);
//
//   vtkm::Normalize(plane_normals[0]);
//   vtkm::Normalize(plane_normals[1]);
//   vtkm::Normalize(plane_normals[2]);
//
//   auto planes
//     = detail::MultiPlane(plane_points, plane_normals, 3);
//   m_internals->m_multi_plane = planes;
// }

//---------------------------------------------------------------------------//
void SliceImplicit::PreExecute()
{
  Filter::PreExecute();
}

//---------------------------------------------------------------------------//
void SliceImplicit::PostExecute()
{
  Filter::PostExecute();
}

//---------------------------------------------------------------------------//
void SliceImplicit::DoExecute()
{

  DataSet data_set;
  const int global_domains = this->m_input->GetGlobalNumberOfDomains();
  if(global_domains == 0)
  {
    // if the number of domains zero there is no work to do
    DataSet *output = new DataSet();
    *output = *(this->m_input);
    this->m_output = output;
    return;
  }

  // TODO: Do we need to do anything special for multi plane?
  
  const int num_domains = this->m_input->GetNumberOfDomains();
  for(int i = 0; i < num_domains; ++i)
  {
    vtkm::Id domain_id;
    vtkm::cont::DataSet dom;
    this->m_input->GetDomain(i, dom, domain_id);

    vtkm::filter::contour::Slice slicer;
    slicer.SetImplicitFunction(m_internals->m_func);
    slicer.SetFieldsToPass(this->GetFieldSelection());
    auto dataset = slicer.Execute(dom);
    data_set.AddDomain(dataset, domain_id);
  }

  CleanGrid cleaner;
  cleaner.SetInput(&data_set);
  cleaner.Update();
  this->m_output = cleaner.GetOutput();
}

std::string
SliceImplicit::GetName() const
{
  return "vtkh::SliceImplicit";
}


//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
// Slice AutoSliceLevels
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
AutoSliceLevels::AutoSliceLevels()
{

}

//---------------------------------------------------------------------------//
AutoSliceLevels::~AutoSliceLevels()
{

}

//---------------------------------------------------------------------------//
void
AutoSliceLevels::SetNormal(vtkm::Vec<vtkm::Float32,3> normal)
{
  m_normals.push_back(normal);
}

//---------------------------------------------------------------------------//
void
AutoSliceLevels::SetLevels(int num_levels)
{
  m_levels = num_levels;
}

//---------------------------------------------------------------------------//
void
AutoSliceLevels::SetField(std::string field)
{
  m_field_name = field;
}

//---------------------------------------------------------------------------//
vtkmCamera*
AutoSliceLevels::GetCamera()
{
  return m_camera;
}

//---------------------------------------------------------------------------//
vtkm::Bounds
AutoSliceLevels::GetDataBounds()
{
  return m_bounds;
}

//---------------------------------------------------------------------------//
vtkm::Vec<vtkm::Float32,3>
AutoSliceLevels::GetNormal()
{
  return m_normal;
}

//---------------------------------------------------------------------------//
vtkm::Float32
AutoSliceLevels::GetRadius()
{
  return m_radius;
}

//---------------------------------------------------------------------------//
void
AutoSliceLevels::PreExecute()
{
  Filter::PreExecute();
}

//---------------------------------------------------------------------------//
vtkm::Vec<vtkm::Float32,3>
GetPoint(int level, int num_levels, vtkm::Bounds bounds)
{
  vtkm::Vec<vtkm::Float32,3> point;
  float spacing = 100.0/num_levels;
  float current_space = level * spacing;
  const float eps = 1e-5;

  float offset = (((current_space) * (4)) / (100)) - 2;
  float t = (offset + 1.f) / 2.f;
  t = std::max(0.f + eps, std::min(1.f - eps, t));
  point[0] = bounds.X.Min + t * (bounds.X.Max - bounds.X.Min);
  point[1] = bounds.Y.Min + t * (bounds.Y.Max - bounds.Y.Min);
  point[2] = bounds.Z.Min + t * (bounds.Z.Max - bounds.Z.Min);

  return point;
}

//---------------------------------------------------------------------------//
void
AutoSliceLevels::DoExecute()
{
  const std::string fname = "slice_field";
  const int num_domains = this->m_input->GetNumberOfDomains();
  const int num_slices = this->m_levels;
  std::string field = this->m_field_name;
  float current_score = -1;
  float winning_score = -1;

  if(num_slices == 0)
  {
    throw Error("AutoSliceLevels: no slice planes specified");
  }
   
  std::vector<float> field_data = vtkh::detail::GetScalarData<float>(*this->m_input, field.c_str());

  float datafield_max = 0.;
  float datafield_min = 0.;

#if ASCENT_MPI_ENABLED
  MPI_Comm mpi_comm = MPI_Comm_f2c(vtkh::GetMPICommHandle());
  float local_datafield_max = 0.;
  float local_datafield_min = 0.;

  if(field_data.size())
  { 
    local_datafield_max = (float)*max_element(field_data.begin(),field_data.end());
    local_datafield_min = (float)*min_element(field_data.begin(),field_data.end());
  }
  MPI_Reduce(&local_datafield_max, &datafield_max, 1, MPI_FLOAT, MPI_MAX, 0, mpi_comm);
  MPI_Reduce(&local_datafield_min, &datafield_min, 1, MPI_FLOAT, MPI_MIN, 0, mpi_comm);

#else

  if(field_data.size())
  { 
    datafield_max = (float)*max_element(field_data.begin(),field_data.end());
    datafield_min = (float)*min_element(field_data.begin(),field_data.end());
  }

#endif

  vtkm::Bounds bounds = this->m_input->GetGlobalBounds();
  vtkm::Vec<vtkm::Float32,3> normal = m_normals[0];
 
  for(int s = 0; s < num_slices; ++s)
  {
    vtkm::Vec<vtkm::Float32,3> point = GetPoint(s, num_slices, bounds);
    vtkh::DataSet temp_ds = *(this->m_input);
    // shallow copy the input so we don't propagate the slice field
    // to the input data set, since it might be used in other places
    for(int i = 0; i < num_domains; ++i)
    {
      vtkm::cont::DataSet &dom = temp_ds.GetDomain(i);

      vtkm::cont::ArrayHandle<vtkm::Float32> slice_field;
      vtkm::worklet::DispatcherMapField<detail::SliceField>(detail::SliceField(point, normal))
        .Invoke(dom.GetCoordinateSystem().GetData(), slice_field);

      dom.AddField(vtkm::cont::Field(fname,
                                      vtkm::cont::Field::Association::Points,
                                      slice_field));
    } // each domain

    vtkh::MarchingCubes marcher;
    marcher.SetInput(&temp_ds);
    marcher.SetIsoValue(0.);
    marcher.SetField(fname);
    marcher.Update();
    
    vtkh::DataSet* output = marcher.GetOutput();
    std::vector<float> slice_data = vtkh::detail::GetScalarData<float>(*output, field.c_str());
    current_score = vtkh::detail::calcEntropyMM<float>(slice_data, slice_data.size(), 256, datafield_min, datafield_max);
    
    if(current_score > winning_score)
    {
      winning_score = current_score;
      this->m_output = output;
    }
  } // each slice
  
  //TODO: needed for setting camera based on input normal
  //if(normal[0] == 1 && normal[1] == 1 &&  normal[2] == 1)
  //{

  //        std::cerr << "normal is 1 1 1 " << std::endl;
  //}
  //else
  //{
  //        std::cerr << "normal is not 1 1 1 " << std::endl;
  //  vtkmCamera *camera = new vtkmCamera;
  //  camera->ResetToBounds(bounds);
  //  std::cerr << "In VTKH Filters" << std::endl;
  //  vtkm::Float32 xb = vtkm::Float32(bounds.X.Length());
  //  vtkm::Float32 yb = vtkm::Float32(bounds.Y.Length());
  //  vtkm::Float32 zb = vtkm::Float32(bounds.Z.Length());
  //  vtkm::Float32 radius = sqrt(xb*xb+yb*yb+zb*zb)/2.0;
  //  std::cerr << "X bounds: " << bounds.X.Min << " " << bounds.X.Max << " ";
  //  std::cerr << "Y bounds: " << bounds.Y.Min << " " << bounds.Y.Max << " ";
  //  std::cerr << "Z bounds: " << bounds.Z.Min << " " << bounds.Z.Max << " ";
  //  std::cerr<<std::endl;
  //  std::cerr << "normal: " << normal[0] << " " << normal[1] << " " << normal[2] << std::endl;
  //  std::cerr << "radius: " << radius << std::endl;

  //  this->m_radius = radius;
  //  this->m_bounds = bounds;
  //  this->m_normal = normal;
  //}
}

//---------------------------------------------------------------------------//
void
AutoSliceLevels::PostExecute()
{
  Filter::PostExecute();
}

//---------------------------------------------------------------------------//
std::string
AutoSliceLevels::GetName() const
{
  return "vtkh::AutoSliceLevels";
}

} // namespace vtkh
