const chai = require("chai");
const expect = chai.expect;
const netcdf4 = require("..");
const { join } = require("path");
const chaiAsPromised = require('chai-as-promised')
chai.use(chaiAsPromised)

const fixture = join(__dirname, "testrh.nc");
const fixture1 = join(__dirname, "test_hgroups.nc");


describe("Dummy test - will be removed",async function() {
  it("should run async open",async ()=>{
    const file=await netcdf4.open('file.nc','w');
    expect(file).to.have.property("name");
    expect(file.name).to.equal("file.nc");
    expect(file).to.have.property("format");
    expect(file.format).to.equal("hdf5");
    expect(file).to.have.property("close");
    expect(file.inspect()).to.equal("[hdf5 file file.nc]");

  });
  it("shold rejected for closing ", async ()=>{
    const file=await netcdf4.open('file.nc','w');
    await expect(file.close()).to.be.rejectedWith("Not implemented yet");
  });
})

describe("File", async function () {
  it("should throw an error when file not found", async function () {
    await expect(netcdf4.open('DOESNOTEXIST','r')).to.rejectedWith("No such file or directory");
  });

  it("should throw an error for wrong file mode",async function () {
    await expect(netcdf4.open('DOESNOTEXIST','wrong mode')).to.rejectedWith("Unknown file mode");
  });

  it("should open a file", async function () {
    const file=await expect(netcdf4.open(fixture, "r")).to.be.fulfilled;
    expect(typeof file).to.eq("object");
    expect(file).not.null;
    expect(file.name).to.eq(fixture);
    expect(file.format).to.eq('netcdf3')
    expect(file.inspect()).to.eq(`[netcdf3 file ${fixture}]`)
    expect(file.closed).false;
    await expect(file.close()).to.be.fulfilled;
    expect(file.inspect()).to.eq(`[Closed netcdf3 file ${fixture}]`)
    expect(file.closed).true;
  });

  it("should open a file (different format)", async function () {
    const file=await expect(netcdf4.open(fixture1, "r")).to.be.fulfilled;
    expect(typeof file).to.eq("object");
    expect(file).not.null;
    expect(file.id).not.null;
    expect(file.name).to.eq(fixture1);
    expect(file.format).to.eq('hdf5')
    expect(file.inspect()).to.eq(`[hdf5 file ${fixture1}]`)
    expect(file.closed).false;
    await expect(file.close()).to.be.fulfilled;
    expect(file.inspect()).to.eq(`[Closed hdf5 file ${fixture1}]`)
    expect(file.closed).true;
  });


  it("file contains variables", async function () {
    const file=await expect(netcdf4.open(fixture, "r")).to.be.fulfilled;
    expect(file).to.have.property('root');
    const variables=await expect(file.root.getVariables()).to.be.fulfilled;
    expect(variables).to.have.property('var1');
    const res=await expect(variables.var1.readSlice(0, 4)).to.be.fulfilled;
    const results = Array.from(res);
    expect(results).to.deep.equal([420, 197, 391.5, 399]);
    await expect(file.close()).to.be.fulfilled;
    expect(file.closed).true;
    expect(file.root).is.undefined;
  });
});
