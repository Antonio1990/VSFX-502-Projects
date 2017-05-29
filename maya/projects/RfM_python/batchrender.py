# batchrender.py
# See also batchRenderRi.mel, batchdatabase.py and batchrif.py
# The base class, BatchDataBase, querries the RMS directory for the current
# project and populates the following database with lists of files and a few
# individual values such as the start and ending frame numbers.
#
#         self.database = {'rootribs' : [], 'geomribs'  : [], 'finalribs': [],
#                           'rmanribs' : [], 'zippedribs': [], 'rlf': [], 
#                         'projpath': project,  'scenename' : scene,
#                         'begin' : int(begin), 'end' : int(end) }
#
# An instance of the BatchRif class returns instances of the rifs the user 
# wishes to apply. They are divided into two categores, those that should be 
# applied to archived geometry ribs and those to be applied to XXX_Final ribs. 
# The actual rif'ing is done by,
#        BatchRif.applyRifsToRibs() and
#        BatchRif.applyRifsToZips()
# 22 Jan 2013 Malcolm Kesson
# 10 Mar 2016 Added the rendering of XXX_DenoiseCrossFrame.job.rib (line 98)
# 23 May 2016 Added sorting the ribs so that renders to "it" are in the correct sequence.
#             Auto rendering a batchrender script that specifies more than 112 frames
#             is problematic. We now force the user to run the script manually.
# 26 May 2016 Because of problems with 32/64bit ctypes on Windows batchrender nolonger
#             supports the running of python rifs on Windows. Rendering on Windows now
#             works OK because of changes to the use of subprocess.
# 31 May 2016 Instead of writing a file called "batchrender" this script now writes
#             a file with the name of the scene appended ex. "batchrender_myscene".
# 30 Nov 2016 If the frame range exceeds 100, for example, frame 1 to frame 245, 
#			  multiple batch render files are generated. For example, on Windows:
#				batchrender_myscene_1_100.bat
#				batchrender_myscene_101_200.bat
#				batchrender_myscene_201_245.bat
#				batchrender_myscene_xframe_denoise.bat  
import re, os, time, sys, subprocess
from batchdatabase import BatchDataBase
  
def main():
	args = sys.argv[1:]
	scene = args[0]     # remove .ma or .mb
	project = args[1]   # full path to project
	begin = args[2]     # start frame
	end = args[3]       # end frame
	layer = args[4]     # render layer, example, "defaultRenderLayer"
	immediate = args[5]
	rifstr = args[6]
	br = BatchRender(scene,project,begin,end,layer,immediate,rifstr)
	
class BatchRender(BatchDataBase):
	DEFAULT_BATCH_NAME = 'batchrender_'
	RIBS_PER_SCRIPT = 100
	#-------------------------------------------------
	# Constructor
	#-------------------------------------------------
	def __init__(self, scene,project,begin,end,layer,immediate,rifstr):
		BatchDataBase.__init__(self, scene,project,begin,end,layer)
		self.project = project
		
		if os.name == "nt" and rifstr != '':
			print('\nERROR: Rib filtering is not supported on Windows')
			print('USE:')
			print('    batchRenderRI("",1,1);')
			print('NOT:')
			print('    batchRenderRI("%s",1,1);' % rifstr)
			return
			
		if rifstr != '':
			# Get an instance of our rif handling class
			from batchrif import BatchRif
			logpath = project
			rifjob = BatchRif(logpath)
			rifs = rifjob.getRifsFromString(rifstr)
			rifs = rifs[1:]  # Ignore the first item
			geomRifs = []
			otherRifs = []
			for rif in rifs:
				if rifjob.isGeometryRif(rif):
					geomRifs.append(rif)
				else:
					otherRifs.append(rif)
			# Static geometry is archived in the shared 'job' directory.
			if len(geomRifs) > 0:
				rifjob.applyRifsToRibs(geomRifs, self.database['geomribs'], 'ascii')
				rifjob.applyRifsToRibs(geomRifs, self.database['rmanribs'], 'ascii')
				# Format for rib-in-.zip will be automatically set to 'binary'
				rifjob.applyRifsToZips(geomRifs, self.database['zippedribs'], self.database['projpath'])
			rifjob.applyRifsToRibs(rifs, self.database['finalribs'], 'ascii')
			
		self.rootribs = self.database['rootribs']
		scriptpath = self.makeBatchRenderScripts()
		doRender = int(immediate)
		# Auto rendering a batchrender script that specifies more than 112 frames
		# is problematic. Force the user to run the script from a terminal...
		if len(self.rootribs) > 100:
			doRender = 0
		if doRender and len(self.rootribs) > 0:
			self.runBatchRenderScript(scriptpath)

 	#-------------------------------------------------
	# getBatchScriptPath
	#-------------------------------------------------	
	def getBatchScriptPath(self, beginAt, total):	
		numRemaining = total - beginAt
		if numRemaining < 0:
			return ""		
		last = total
		if numRemaining >= BatchRender.RIBS_PER_SCRIPT:
			last = beginAt + BatchRender.RIBS_PER_SCRIPT - 1
		else:
			last = total
		scriptname = BatchRender.DEFAULT_BATCH_NAME + self.database['scenename']
		scriptname += '_%d_%d' % (beginAt, last)
		if os.name == "nt":
			scriptname += '.bat'
		batchpath = os.path.join(self.database['projpath'], scriptname)
		return batchpath
				
 	#-------------------------------------------------
	# makeBatchRenderScripts
	#-------------------------------------------------
	# Writes one or more batch render scripts. In the case of multiple scripts
	# each contains 100 lines of "prman -t:all PATH_TO_RIB" statements. If 
	# "immediate render mode" is ON the first script will automatically run but
	# other scripts must be run manually. 
	def makeBatchRenderScripts(self):
		if len(self.database['projpath']) > 0:
			rootdir = self.database['projpath']
			if os.name == "nt":
				rootdir = self.convertToWindows(self.project)
				rootdir = rootdir.rstrip('\\')
			else:
				rootdir = rootdir.rstrip('/')
		binpath = os.getenv('RMANTREE')
		binpath = os.path.join(binpath, 'bin')
		prmanpath = os.path.join(binpath, 'prman')
		if os.name == "nt":
			prmanpath = self.convertToWindows(prmanpath)
				
		# sort the ribs
		ribs = self.database['rootribs']
		ribs.sort()
		
		# Nov.16.breaking into multiple scripts
		batchpath = self.getBatchScriptPath(1, len(ribs))
		
		f = open(batchpath, 'w')
		if os.name == "nt":
			f.write('\n')
		else:
			f.write('export RMANTREE=%s\n' % os.getenv('RMANTREE'))
		rib_counter = 1
		num_batchscripts = 1
		for rib in ribs:
			if os.name == "nt":
				rib = self.convertToWindows(rib)
			f.write('"' + prmanpath + '" -cwd "' + rootdir + '" -t:all "' + rib + '"\n')
			if rib_counter % BatchRender.RIBS_PER_SCRIPT == 0:	
				path = self.getBatchScriptPath(rib_counter + 1,len(ribs))
				if len(path) > 0:
					f.close()
					f = open(path, 'w')
					if os.name == "nt":
						f.write('\n')
					else:
						f.write('export RMANTREE=%s\n' % os.getenv('RMANTREE'))
					num_batchscripts += 1
			rib_counter += 1
		# Look for a frame denoise rib, for example, 'perspShape_Denoise.0002.rib'
		if len(ribs) == 1:
			denoise_rib = ''
			framedir = os.path.dirname(ribs[0])
			frame_ribs = os.listdir(framedir)
			for frame_rib in frame_ribs:
				name = os.path.basename(frame_rib)
				clipped = name[:-4]
				if '_Denoise' in clipped:
					denoise_rib = name
			if len(denoise_rib) > 0:
				denoise_rib_path = os.path.join(framedir,denoise_rib)
				if os.name == "nt":
					denoise_rib_path = self.convertToWindows(denoise_rib_path)
				f.write('"' + prmanpath + '" -cwd "' + rootdir + '" -t:all "' + denoise_rib_path + '"\n')
				
		else: # Look for a crossframe rib, for example, 'perspShape_DenoiseCrossFrame_MasterLayer.job.rib'
			denoise_jobrib = ''
			jobdir = os.path.join(rootdir,'renderman',self.database['scenename'],'rib','job')
			job_ribs = os.listdir(jobdir)
			for job_rib in job_ribs:
				name = os.path.basename(job_rib)
				clipped = name[:-8]
				if '_DenoiseCrossFrame' in clipped:
					denoise_jobrib = name
			if len(denoise_jobrib) > 0:
				# If there are multiple batch render scripts we "put" the denoise into a separate 
				# batch render script.
				if num_batchscripts > 1:
					f.close()
					denoise_batchpath = BatchRender.DEFAULT_BATCH_NAME + self.database['scenename'] + '_xframe_denoise'
					denoise_batchpath = os.path.join(self.database['projpath'], denoise_batchpath)
					if os.name == "nt":
						denoise_batchpath += '.bat'
					f = open(denoise_batchpath, 'w')
					if os.name == "nt":
						f.write('\n')
					else:
						f.write('export RMANTREE=%s\n' % os.getenv('RMANTREE'))
				f.write('"' + prmanpath + '" -cwd "' + rootdir + '" -t:all "' + os.path.join(jobdir,denoise_jobrib) + '"\n')
		f.close()
		return batchpath
  
	#-------------------------------------------------
	# runBatchRenderScript
	#-------------------------------------------------
	def runBatchRenderScript(self, fullpath):
		if os.name == "posix":
			os.chmod(fullpath, 0777)
			if sys.platform == 'darwin': # MacOSX
				args = ['open', fullpath]
			else:    # linux
				args = ['sh', fullpath]
			subprocess.Popen(args,stdout=subprocess.PIPE)
		else:    # windows
			args = ['start', fullpath]
			subprocess.call(args, shell=True)
		
	def log(self, logname, logMsg):
		f = open(os.path.join(self.project, logname), 'w')
		localtime = time.asctime( time.localtime(time.time()) )
		f.write('Time %s.\n' % localtime)
		f.write(logMsg)
		f.close()            
		
	def print_database(self):
		for rib in self.database['rootribs']:
			print '--------------rootribs------------'
			print rib
		print '--------------geomribs------------'
		for rib in self.database['geomribs']:
			print rib
		print '--------------finalribs------------'
		for rib in self.database['finalribs']:
			print rib
		print '--------------rmanribs------------'
		for rib in self.database['rmanribs']:
			print rib
		print '--------------zippedribs------------'
		for rib in self.database['zippedribs']:
			print rib
		print '--------------rlf------------'
		for rib in self.database['rlf']:
			print rib
			
	# Utility_____________________________________________    
	def convertToWindows(self, linuxpath):
		pattern = re.compile(r"/")
		return pattern.sub(r'\\', linuxpath)
  

