/*
   Source File : DocumentContext.h


   Copyright 2011 Gal Kahana PDFWriter

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   
*/
#pragma once

#include "EStatusCode.h"
#include "EPDFVersion.h"
#include "IOBasicTypes.h"
#include "TrailerInformation.h"
#include "CatalogInformation.h"
#include "JPEGImageHandler.h"
#include "TIFFImageHandler.h"
#include "TIFFUsageParameters.h"
#include "UsedFontsRepository.h"
#include "PDFEmbedParameterTypes.h"
#include "PDFDocumentHandler.h"

#include <string>
#include <set>
#include <utility>

using namespace std;
using namespace IOBasicTypes;

class DictionaryContext;
class ObjectsContext;
class PDFPage;
class PageTree;
class OutputFile;
class IDocumentContextExtender;
class PageContentContext;
class ResourcesDictionary;
class PDFFormXObject;
class PDFRectangle;
class PDFImageXObject;
class PDFUsedFont;
class PageContentContext;

typedef set<IDocumentContextExtender*> IDocumentContextExtenderSet;

class DocumentContext
{
public:
	DocumentContext();
	~DocumentContext();

	void SetObjectsContext(ObjectsContext* inObjectsContext);
	void SetOutputFileInformation(OutputFile* inOutputFile);
	EStatusCode	WriteHeader(EPDFVersion inPDFVersion);
	EStatusCode	FinalizePDF();

	TrailerInformation& GetTrailerInformation();
	CatalogInformation& GetCatalogInformation();

	// Page and Page Content Writing

	// Use StartPageContentContext for creating a page content for this page.
	// using the context you can now start creating content for it.
	// if this page already has a content context, it will be returned
	PageContentContext* StartPageContentContext(PDFPage* inPage);

	// Use PausePageContentContext in order to flush a current content write, in order
	// to write some other object. A common use case is when wanting to use an image in a page.
	// first the content before the image is written, then the content is paused, a new object that represents the image
	// is written, and then the content continues showing the image with a "do" operator. This is also the cause for creating multiple content
	// streams for a page (and what will happen in this implementation as well).
	EStatusCode PausePageContentContext(PageContentContext* inPageContext);

	// Finalize and release the page context. the current content stream is flushed to the PDF stream
	EStatusCode EndPageContentContext(PageContentContext* inPageContext);
	
	EStatusCodeAndObjectIDType WritePage(PDFPage* inPage);
	EStatusCodeAndObjectIDType WritePageAndRelease(PDFPage* inPage);


	// Form XObject creation and finalization
	PDFFormXObject* StartFormXObject(const PDFRectangle& inBoundingBox,const double* inMatrix = NULL);
	PDFFormXObject* StartFormXObject(const PDFRectangle& inBoundingBox,ObjectIDType inFormXObjectID,const double* inMatrix = NULL);
	EStatusCode EndFormXObjectAndRelease(PDFFormXObject* inFormXObject);

	// no release version of ending a form XObject. owner should delete it (regular delete...nothin special)
	EStatusCode EndFormXObjectNoRelease(PDFFormXObject* inFormXObject);

	// Image XObject creating. 
	// note that as oppose to other methods, create the image xobject also writes it, so there's no "WriteXXXXAndRelease" for image.
	// So...release the object yourself [just delete it]

	// JPEG - two variants
	
	// will return image xobject sized at 1X1
	PDFImageXObject* CreateImageXObjectFromJPGFile(const wstring& inJPGFilePath);
	PDFImageXObject* CreateImageXObjectFromJPGFile(const wstring& inJPGFilePath,ObjectIDType inImageXObjectID);

	// will return form XObject, which will include the xobject at it's size
	PDFFormXObject* CreateFormXObjectFromJPGFile(const wstring& inJPGFilePath);
	PDFFormXObject* CreateFormXObjectFromJPGFile(const wstring& inJPGFilePath,ObjectIDType inFormXObjectID);

	// TIFF
	PDFFormXObject* CreateFormXObjectFromTIFFFile(	const wstring& inTIFFFilePath,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters);

	PDFFormXObject* CreateFormXObjectFromTIFFFile(	const wstring& inTIFFFilePath,
													ObjectIDType inFormXObjectID,
													const TIFFUsageParameters& inTIFFUsageParameters = TIFFUsageParameters::DefaultTIFFUsageParameters);
	
	// PDF
	EStatusCodeAndPDFFormXObjectList CreateFormXObjectsFromPDF( const wstring& inPDFFilePath,
																const PDFPageRange& inPageRange,
																EPDFPageBox inPageBoxToUseAsFormBox,
																const double* inTransformationMatrix);

	EStatusCodeAndObjectIDTypeList AppendPDFPagesFromPDF(const wstring& inPDFFilePath,
														const PDFPageRange& inPageRange);


	// Font [Text]
	PDFUsedFont* GetFontForFile(const wstring& inFontFilePath);
	// second overload is for type 1, when an additional metrics file is available
	PDFUsedFont* GetFontForFile(const wstring& inFontFilePath,const wstring& inAdditionalMeticsFilePath);

	// Extensibility
	void AddDocumentContextExtender(IDocumentContextExtender* inExtender);
	void RemoveDocumentContextExtender(IDocumentContextExtender* inExtender);

	// JPG images handler for retrieving JPG images information
	JPEGImageHandler& GetJPEGImageHandler();

	
private:
	ObjectsContext* mObjectsContext;
	TrailerInformation mTrailerInformation;
	CatalogInformation mCatalogInformation;
	wstring mOutputFilePath;
	IDocumentContextExtenderSet mExtenders;
	JPEGImageHandler mJPEGImageHandler;
	TIFFImageHandler mTIFFImageHandler;
	PDFDocumentHandler mPDFDocumentHandler;
	UsedFontsRepository mUsedFontsRepository;
	
	void WriteHeaderComment(EPDFVersion inPDFVersion);
	void Write4BinaryBytes();
	EStatusCode WriteCatalogObject();
	EStatusCode WriteTrailerDictionary();
	void WriteXrefReference(LongFilePositionType inXrefTablePosition);
	void WriteFinalEOF();
	void WriteInfoDictionary();
	void WritePagesTree();
	int WritePageTree(PageTree* inPageTreeToWrite);
	string GenerateMD5IDForFile();
	EStatusCode WriteResourcesDictionary(ResourcesDictionary& inResourcesDictionary);
	bool IsIdentityMatrix(const double* inMatrix);
	EStatusCode WriteUsedFontsDefinitions();
};
