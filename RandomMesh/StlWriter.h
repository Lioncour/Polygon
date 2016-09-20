#pragma once

using namespace Windows::Storage;

namespace RandomMesh
{
	private ref class StlWriter sealed
	{
	internal:
		static void Save(Streams::DataWriter^ dataWriter, shared_ptr<Mesh> mesh)
		{
			if (dataWriter == nullptr)
			{
				throw ref new Platform::InvalidArgumentException();
			}

			if (!mesh)
			{
				throw ref new Platform::InvalidArgumentException();
			}

			auto vertices = mesh->GetVertices();
			auto indices = mesh->GetIndices();

			uint32_t trianglesCount = 1;//indices.size() / 3;

			WriteHeader(dataWriter, trianglesCount);

			for (size_t index = 0; index < vertices.size(); index += 3)
			{
				auto a = vertices[indices[index + 0]];
				auto b = vertices[indices[index + 1]];
				auto c = vertices[indices[index + 2]];

				AddTriangle(dataWriter, a, b, c);

				break;
			}
		}

	private:
		static void WriteHeader(Streams::DataWriter^ dataWriter, uint32_t trianglesCount)
		{
			dataWriter->WriteByte(0x46);
			dataWriter->WriteByte(0x49);
			dataWriter->WriteByte(0x47);
			dataWriter->WriteByte(0x55);
			dataWriter->WriteByte(0x52);
			dataWriter->WriteByte(0x45);
			
			dataWriter->WriteUInt32(trianglesCount);
		}

		static void AddTriangle(Streams::DataWriter^ dataWriter, VertexPositionColor a, VertexPositionColor b, VertexPositionColor c)
		{
			AddVector(dataWriter, a.normal);
			AddVector(dataWriter, a.pos);
			AddVector(dataWriter, b.pos);
			AddVector(dataWriter, c.pos);
			dataWriter->WriteUInt16(GetColor(a.color, b.color, c.color));			
		}

		static void AddVector(Streams::DataWriter^ dataWriter, DirectX::XMFLOAT3 vector)
		{
			dataWriter->WriteSingle(vector.x);
			dataWriter->WriteSingle(vector.y);
			dataWriter->WriteSingle(vector.z);
		}

		static uint16_t GetColor(DirectX::XMFLOAT3 a, DirectX::XMFLOAT3 b, DirectX::XMFLOAT3 c)
		{
			auto rc = static_cast<uint8_t>((a.x + b.x + c.x) / 3 * 31);
			auto gc = static_cast<uint8_t>((a.y + b.y + c.y) / 3 * 31);
			auto bc = static_cast<uint8_t>((a.z + b.z + c.z) / 3 * 31);

			return static_cast<uint16_t>(
				0x8000
				| (rc & 0x1F) << 10
				| (gc & 0x1F) << 5
				| (bc & 0x1F) << 0);
		}
	};
}

//public class STLWriter
//{
//	static final int DEFAULT_RGB = 0xFF8F8F8F; // light gray
//
//	private OutputStream os;                // output stream 
//	private byte[] buffer = new byte[84];   // write buffer
//	private int bi;                         // current buffer index
//	private int bufferSize;                 // size of file write buffer 
//	protected int numTriangles;             // starting triangle count
//	protected int addedTriangles;           // number of added triangles
//
//											/** constructor with a default buffer size of 64 kByte
//											*/
//	public STLWriter()
//	{
//		this.bufferSize = 0x10000;      // 64 kByte
//	}
//
//	/** constructor using given buffersize
//	* @param  bufferSize  buffer size in bytes
//	*/
//	public STLWriter(int bufferSize)
//	{
//		this.bufferSize = bufferSize;
//	}
//
//	/** open STL file stream for adding triangle faces
//	* @param stream     output stream
//	* @param numFaces   number of scenery faces
//	* @param title      scenery title
//	*/
//	public void beginSave(OutputStream stream, int numFaces, String title)
//	{
//		try
//		{
//			os = new BufferedOutputStream(new DataOutputStream(stream), bufferSize);
//			writeHeader(title, numFaces);
//		}
//		catch (Exception e)
//		{
//			e.printStackTrace();
//		}
//	}
//
//	/** open STL file for adding triangle faces
//	* @param filename   name of STL file (*.stl)
//	* @param numFaces   number of scenery faces
//	* @param title      scenery title
//	*/
//	public void beginSave(String filename, int numFaces, String title)
//	{
//		numTriangles = numFaces;
//		addedTriangles = 0;
//		try
//		{
//			beginSave(new FileOutputStream(filename), numFaces, title);
//		}
//		catch (Exception e)
//		{
//			e.printStackTrace();
//		}
//	}
//
//	/** close STL file */
//	public void endSave()
//	{
//		try
//		{
//			os.close();
//		}
//		catch (Exception e)
//		{
//			e.printStackTrace();
//		}
//		if (numTriangles != addedTriangles)
//			println("WARNING: started with " + numTriangles + " faces, but " + addedTriangles + " added!");
//	}
//
//	/** save triangle a,b,c with default color and default normal vector
//	* @param      a  triangle vertex 1
//	* @param      b  triangle vertex 2
//	* @param      c  triangle vertex 3
//	*/
//	public void addTriangle(PVector a, PVector b, PVector c)
//	{
//		addTriangle(a, b, c, DEFAULT_RGB);
//	}
//
//	/** save triangle a,b,c with color rgb and default normal vector
//	* @param      a  triangle vertex 1
//	* @param      b  triangle vertex 2
//	* @param      c  triangle vertex 3
//	* @param    rgb  24 bit color with RGB format
//	*/
//	public void addTriangle(PVector a, PVector b, PVector c, int rgb)
//	{
//		PVector nn = new PVector();
//		PVector u = new PVector(b.x, b.y, b.z);
//		PVector v = new PVector(c.x, c.y, c.z);
//		u.sub(a);
//		v.sub(a);
//		nn = u.cross(v);  // get triangle normal
//		nn.normalize();
//		//println ("n=" +n);
//		addTriangle(a, b, c, rgb, nn);
//	}
//
//	/** save triangle a,b,c with color rgb and given normal vector
//	* @param      a  triangle vertex 1
//	* @param      b  triangle vertex 2
//	* @param      c  triangle vertex 3
//	* @param    rgb  24 bit color with RGB format
//	* @param normal  normalized normal vector of triangle
//	*/
//	public void addTriangle(PVector a, PVector b, PVector c, int rgb, PVector normal)
//	{
//		bi = 0;   // reset buffer index
//		short shortRGB = Color2RGB15(rgb);
//		//println ("rgb=" +hex(rgb) + "  s=" +hex(shortRGB));
//		try
//		{
//			addVector(normal);          // triangle normal
//			addVector(a);               // triangle vertices
//			addVector(b);
//			addVector(c);
//			addShortInteger(shortRGB);  // triangle short color
//			os.write(buffer, 0, 50);   // write triangle data
//			addedTriangles++;           // count added triangles
//		}
//		catch (Exception e)
//		{
//			e.printStackTrace();
//		}
//	}
//
//
//	//=== private methods ===  
//
//	// write STL file header
//	private void writeHeader(String title, int numFaces) throws IOException
//	{
//		int len = min(79, title.length());
//		for (int ni = 0; ni <= len; ni++)
//			if (ni == len)
//				buffer[ni + 1] = 0;
//			else buffer[ni] = (byte)title.charAt(ni);
//
//			bi = 80;   // set buffer index
//			addInteger(numFaces);
//			os.write(buffer, 0, 84);
//	}
//
//	// add 2 byte integer value 
//	private void addShortInteger(short value)
//	{
//		buffer[bi++] = (byte)(value & 0xff);
//		buffer[bi++] = (byte)(value >> 8 & 0xff);
//	}
//
//	// add 4 byte integer value 
//	private void addInteger(int value)
//	{
//		buffer[bi++] = (byte)(value & 0xff);
//		buffer[bi++] = (byte)(value >> 8 & 0xff);
//		buffer[bi++] = (byte)(value >> 16 & 0xff);
//		buffer[bi++] = (byte)(value >> > 24);
//	}
//
//	// add 4 byte float value 
//	private void addFloat(float value)
//	{
//		addInteger(Float.floatToRawIntBits(value));
//		// see http://docs.oracle.com/javase/7/docs/api/java/lang/Float.html
//	}
//
//	// add vector with 4 byte x,y,z float values  
//	private void addVector(PVector v)
//	{
//		addFloat(v.x);
//		addFloat(v.y);
//		addFloat(v.z);
//	}
//
//	//---------------------------------------------------------
//	// convert 32bit color to word color with 1555 format
//	// input:     color        argb values (0..255)
//	// return:    word         16 bit color (1+3*5 bit)
//	// format:    32bit        aaaaaaaarrrrrrrrggggggggbbbbbbbb 
//	//            16bit (1555) 1rrrrrgg gggbbbbb
//	//---------------------------------------------------------
//	private short Color2RGB15(color aColor)
//	{
//		return (short)(0x8000
//			| (aColor & 0xF80000) >> 9      // red 
//			| (aColor & 0x00F800) >> 6      // green
//			| (aColor & 0x0000F8) >> 3);    // blue
//	}
//
//}