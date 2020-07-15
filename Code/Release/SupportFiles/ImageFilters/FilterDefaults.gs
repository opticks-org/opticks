<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<FilterList xmlns="https://comet.balldayton.com/standards/namespaces/2005/v1.1/comet.xsd">

   <filter name="EdgeDetection" type="ImageFilter"
     description="This program is used to compute an edge detection filter.">
      <program name="EdgeDetection.glsl" type="Fragment">
         <parameter name="inputImage" type="unsigned int" value="0"></parameter>
      </program>
   </filter>

   <filter name="ByPass" type="ImageFilter"
     description="This program is used during testing of the the GPU read back capability.">
      <program name="ByPass.glsl" type="Fragment">
         <parameter name="inputImage" type="unsigned int" value="0"></parameter>
      </program>
   </filter>

   <filter name="Bloom" type="ImageFilter"
     description="Example Filter.">
      <program name="bloom.glsl" type="Fragment">
         <parameter name="inputImage" type="unsigned int" value="0"></parameter>
      </program>
   </filter>

   <filter name="Median3x3" type="ImageFilter"
     description="This program is an example 3x3 median filter.">
      <program name="Median.glsl" type="Fragment">
         <parameter name="inputImage" type="unsigned int" value="0"></parameter>
      </program>
   </filter>

   <filter name="GaussFilter" type="ImageFilter"
     description="This program is an example gauss filter.">
      <program name="GaussFilter.glsl" type="Fragment">
         <parameter name="inputImage" type="unsigned int" value="0"></parameter>
      </program>
   </filter>

   <filter name="LaplacianEdges" type="ImageFilter"
     description="This program is an example LaplacianEdges filter.">
      <program name="LaplacianEdges.glsl" type="Fragment">
         <parameter name="inputImage" type="unsigned int" value="0"></parameter>
      </program>
   </filter>

   <filter name="Sharpen" type="ImageFilter"
     description="This program is an example sharpen filter.">
      <program name="Sharpen.glsl" type="Fragment">
         <parameter name="inputImage" type="unsigned int" value="0"></parameter>
      </program>
   </filter>

   <filter name="Background" type="FeedbackFilter"
     description="This program is an example feedback filter.">
      <program name="Feedback.glsl" type="Fragment">
         <parameter name="inputImage" type="unsigned int" value="0"></parameter>
		 <parameter name="estimateImage" type="unsigned int" value="0"></parameter>
      </program>
      <program name="BackgroundEstimate.glsl" type="Fragment">
         <parameter name="inputImage" type="unsigned int" value="0"></parameter>
		 <parameter name="filteredImage" type="unsigned int" value="0"></parameter>
		 <parameter name="coefficient" type="float" value="0"></parameter>
      </program>
   </filter>

</FilterList>
