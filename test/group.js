const chai = require("chai");
const expect = chai.expect;
const chaiAsPromised = require('chai-as-promised');
chai.use(chaiAsPromised);

const netcdf4 = require("..");
const { copyFileSync, unlinkSync } = require("fs");
const { tmpdir } = require("os");
const { join } = require("path");

const fixture = join(__dirname, "test_hgroups.nc");

describe("Group", function () {
  let file,tempFileName;

  beforeEach(async function () {
    tempFileName = join(tmpdir(), `${Date.now()}.rc`)
    copyFileSync(fixture, tempFileName);
    file = await netcdf4.open(tempFileName, "w");
  });

  afterEach(async function () {
  try{
    await file.close();
    unlinkSync(tempFileName);
  } catch (ignore) {
    console.log(`Got ${ignore.message} during file ${file?file:"undefined"} closing, ingored`)
  }
  });

  it("should read name", async  function () {
    const realname=await expect(file.root.getName()).to.be.fulfilled;
    expect(realname,'/');
  });

  it("should read group path", async function () {
    const realname=await expect(file.root.getPath()).to.be.fulfilled;
    expect(realname).to.equal("/");
  });

  it("has custom inspection", function () {
    expect(file.root.inspect()).eq("[Group /]");
  });

  it("should read list of subgroups", async function () {
    // const subgroups = await expect(file.root.getSubgroups()).to.be.fulfilled;
    const subgroups = await file.root.getSubgroups();
    expect(subgroups).to.have.property(
      "mozaic_flight_2012030319051051_descent"
    );
    expect(subgroups).to.have.property(
      "mozaic_flight_2012030319051051_descent"
    );
    expect(subgroups).to.have.property(
      "mozaic_flight_2012030321335035_descent"
    );
    expect(subgroups).to.have.property("mozaic_flight_2012030403540535_ascent");
    expect(subgroups).to.have.property(
      "mozaic_flight_2012030403540535_descent"
    );
    expect(subgroups).to.have.property("mozaic_flight_2012030412545335_ascent");
    expect(subgroups).to.have.property("mozaic_flight_2012030419144751_ascent");
  });


  it("should read name/fullname/subgroup of subgroups", async function () {
    const subgroups = await expect(file.root.getSubgroups()).to.be.fulfilled;
    expect(subgroups.mozaic_flight_2012030419144751_ascent.inspect(),'[Group mozaic_flight_2012030419144751_ascent]');
    await expect(subgroups.mozaic_flight_2012030419144751_ascent.getName()).become('mozaic_flight_2012030419144751_ascent');
    await expect(subgroups.mozaic_flight_2012030419144751_ascent.getPath()).become('/mozaic_flight_2012030419144751_ascent');
  });

  it("should get subgroup", async function () {
    const subgroup = await expect(file.root.getSubgroup("mozaic_flight_2012030419144751_ascent")).to.be.fulfilled;
    expect(subgroup.inspect(),'[Group mozaic_flight_2012030419144751_ascent]');
    await expect(subgroup.getName()).become('mozaic_flight_2012030419144751_ascent');
    await expect(subgroup.getPath()).become('/mozaic_flight_2012030419144751_ascent');
  });

  it("getSubgroup(..) should throw with unexisting subgroup", async function () {
    const subgroup = await expect(file.root.getSubgroup("no-exist")).to.rejectedWith("NetCDF4: Bad or missing group \"no-exist\"");
  });

  it("getSubgroup(..) should throw when parameters missing", async function () {
    const subgroup = await expect(file.root.getSubgroup()).to.rejectedWith("Expected subgroup name");
  });


  it("should rename groups", async function () {
    var subgroups = await expect(file.root.getSubgroups()).to.be.fulfilled;
    var subgroup = subgroups.mozaic_flight_2012030419144751_ascent;
    await expect(subgroup.setName('new_mozaic_flight')).become('new_mozaic_flight');
    await expect(subgroup.getName()).become('new_mozaic_flight');
    await expect(subgroup.getPath()).become('/new_mozaic_flight');
    await file.close();
    file = await netcdf4.open(tempFileName, "r");
    var subgroups = await expect(file.root.getSubgroups()).to.be.fulfilled;
    var subgroup = subgroups.new_mozaic_flight;
    await expect(subgroup.getName()).become('new_mozaic_flight');
    await expect(subgroup.getPath()).become('/new_mozaic_flight');

  });

  it("should create new group",async function() {
    var newGroup=await expect(file.root.addSubgroup("new_group")).be.fulfilled;
    await expect(newGroup.getName()).become('new_group');
    await expect(newGroup.getPath()).become('/new_group');
    await expect(file.root.getSubgroups()).eventually.to.have.property("new_group");
    var newSubGroup=await expect(newGroup.addSubgroup("new_nested_group")).be.fulfilled;
    await expect(newSubGroup.getName()).become('new_nested_group');
    await expect(newSubGroup.getPath()).become('/new_group/new_nested_group');
    await expect(newGroup.getSubgroups()).eventually.to.have.property("new_nested_group");
    await file.close();
    file = await new netcdf4.open(tempFileName, "r");
    var subgroup = (await file.root.getSubgroups()).new_group;
    await expect(subgroup.getName()).become('new_group');
    await expect(subgroup.getPath()).become('/new_group');
    await expect(subgroup.getSubgroups()).eventually.to.have.property("new_nested_group");

  })

  it("should read list of dimensions", async function () {
    await expect(file.root.getDimensions()).eventually.to.have.property("recNum");
  });

  it("should add new dimension",async function() {
    await expect(file.root.getDimensions()).eventually.to.not.have.property("new_dim");
    await expect(file.root.addDimension("new_dim",10)).eventually.to.be.deep.eq({"new_dim":10});
    await expect(file.root.getDimensions()).eventually.to.have.property("new_dim").to.be.deep.eq(10);
    await file.close();
    file = await netcdf4.open(tempFileName, "r");
    await expect(file.root.getDimensions()).eventually.to.have.property("new_dim").to.be.deep.eq(10);
  });

  it("should add new unlimited dimension",async function() {
    await expect(file.root.getDimensions()).eventually.to.not.have.property("new_dim");
    await expect(file.root.getDimensions(true)).eventually.to.not.have.property("new_dim");
    await expect(file.root.addDimension("new_dim",'unlimited')).eventually.to.be.deep.eq({"new_dim":"unlimited"});
    await expect(file.root.getDimensions()).eventually.to.have.property("new_dim").to.be.deep.eq("unlimited");
    await expect(file.root.getDimensions(true)).eventually.to.have.property("new_dim").to.be.deep.eq("unlimited");
    await file.close();
    file = await netcdf4.open(tempFileName, "r");
    await expect(file.root.getDimensions()).eventually.to.have.property("new_dim").to.be.deep.eq("unlimited");
    await expect(file.root.getDimensions(true)).eventually.to.have.property("new_dim").to.be.deep.eq("unlimited");
  });

  it("should read list of variables", async function () {
    await expect(file.root.getVariables()).eventually.to.have.property("UTC_time");
  });

  it("should create new variable ", async function () {
    await expect(file.root.getVariables()).eventually.to.not.have.property("test_variable");
    var newVar=await expect(file.root.addVariable('test_variable','byte',['recNum'])).be.fulfilled;
    expect(newVar.inspect(),'[Variable test_variable, type byte, 1 dimension(s)]');
    await expect(file.root.getVariables()).eventually.to.have.property("test_variable");
    var vars=await file.root.getVariables();
    expect(vars.test_variable.inspect(),'[Variable test_variable, type byte, 1 dimension(s)]');
    await file.close();
    file = await netcdf4.open(tempFileName, "r");
    await expect(file.root.getVariables()).eventually.to.have.property("test_variable");
    vars=await file.root.getVariables();
    expect(vars.test_variable.inspect(),'[Variable test_variable, type byte, 1 dimension(s)]');
  });

  it("should read list of attributes", async function () {
    const attributes =
      await expect(
        file.root.getSubgroup("mozaic_flight_2012030419144751_ascent").then(group=>group.getAttributes()))
        .to.be.fulfilled;
    expect(attributes).to.have.property("airport_dep");
    expect(attributes).to.have.property("flight");
    expect(attributes).to.have.property("level");
    expect(attributes).to.have.property("airport_arr");
    expect(attributes).to.have.property("mission");
    expect(attributes).to.have.property("time_dep");
    expect(attributes).to.have.property("aircraft");
    expect(attributes).to.have.property("link");
    expect(attributes).to.have.property("phase");
    expect(attributes).to.have.property("time_arr");
  });

  it("should add attribute",async function() {
    await expect(file.root.getAttributes()).eventually.to.not.have.property("root_attr_prop");
    const attr=await expect(file.root.addAttribute("root_attr_prop","string","root attr property")).to.be.fulfilled;
    expect(attr).deep.equal({"root_attr_prop":{"type":"string","value":"root attr property"}});
    expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop");
    await file.close();
    file = await netcdf4.open(tempFileName, "r");
    await expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop");
    await expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop").to.deep.property("value").to.deep.equal("root attr property");
    await expect(file.root.getAttributes(true)).eventually.to.have.property("root_attr_prop").to.deep.equal({"type":"string","value":"root attr property"});
  })  

  // it("should add attribute",async function() {
  //   await expect(file.root.getAttributes()).eventually.to.not.have.property("root_attr_prop");
  //   const attr = await expect(file.root.addAttribute("root_attr_prop","string","root attr property")).to.be.fulfilled;
  //   console.log(attr);
  //   await expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop");
  //   await file.close();
  //   file = await netcdf4.open(tempFileName, "r");
  //   await expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop");
  //   await expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop").to.deep.property("value").to.deep.equal("root attr property");
  //   await expect(file.root.getAttributes(true)).eventually.to.have.property("root_attr_prop").to.deep.equal({"type":"string","value":"root attr property"});
  // })  


});
