# Some example methods for a rif

import prman
class Rif(prman.Rif):
	def __init__(self, ri):
		prman.Rif.__init__(self, ri)

	def Attribute(self, attr, params):
		# To get the name of an item
		if attr == 'identifier':
			name = params['string name'][0]
		self.m_ri.Attribute(attr, params)

	def Blobby(self, numblobs, opcodes, xyz, strs, params):
		self.m_ri.Blobby(numblobs, opcodes, xyz, strs, params)
	
	def Color(self, r,g,b):
		self.m_ri.Color( (r,g,b) )

	def ConcatTransform(self,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p):
		self.m_ri.ConcatTransform([a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p])

	def Displacement(self, path, params):
		self.m_ri.Displacement(path, params)

	def Display(self, name, driver, channels, params):
		#print params.keys()
		self.m_ri.Display(name, driver, channels, params)

	def Interior(self, path, params):
		self.m_ri.Interior(path, params)
		
	def Opacity(self, r,g,b):
		self.m_ri.Opacity( (r,g,b) )

	def Points(self, num_points, params): # two args in
		#print params['constantwidth']
		self.m_ri.Points(params) # one arg out

	def PointsGeneralPolygons(self, nloops, nverts, verts, params):
		#print params['P']
		self.m_ri.PointsGeneralPolygons(nloops, nverts, verts, params)

	def Procedural(self, data, bounds, proctype):
		# Example proctypes are 
		#    'ProcDelayedReadArchive', 
		#    'ProcDynamicLoad' and,
		#    'ProcRunProgram'
		self.m_ri.Procedural(data, bounds, proctype)

	def ReadArchive(self, ribfile):
		self.m_ri.ReadArchive(ribfile)

	def Surface(self, path, params):
		self.m_ri.Surface(path, params)
			
	def VArchiveRecord(self, record_type, text):
		# Using a newline enables statements to be added to the rib stream
		#self.m_ri.VArchiveRecord('comment', '\nThis is not a comment!!')
		self.m_ri.VArchiveRecord(record_type, text)


	

